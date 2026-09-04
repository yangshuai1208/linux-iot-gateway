#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <pthread.h>

#define QUEUE_CAPACITY 4U
#define ITEM_COUNT 10

typedef struct
{
    int data[QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
    bool finished;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} IntQueue;

typedef struct
{
    IntQueue *queue;
    int sum;
    int consumed_count;
} ConsumerContext;

static bool QueueInit(IntQueue *queue)
{
    if (queue == NULL)
    {
        return false;
    }

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
    queue->finished = false;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        return false;
    }

    if (pthread_cond_init(&queue->not_empty, NULL) != 0)
    {
        pthread_mutex_destroy(&queue->mutex);
        return false;
    }

    if (pthread_cond_init(&queue->not_full, NULL) != 0)
    {
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->mutex);
        return false;
    }

    return true;
}

static void QueueDestroy(IntQueue *queue)
{
    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);
    pthread_mutex_destroy(&queue->mutex);
}

static bool QueuePush(IntQueue *queue, int value)
{
    if (queue == NULL)
    {
        return false;
    }

    /*
     * TODO 1：
     * 1. 加锁
     * 2. 队列满时，使用not_full条件变量等待
     * 3. 将value写入tail位置
     * 4. tail循环移动，count加1
     * 5. 通知等待not_empty的消费者
     * 6. 解锁
     */
    pthread_mutex_lock(&queue->mutex);
    while(queue->count==QUEUE_CAPACITY)
    {
        pthread_cond_wait(&queue->not_full,&queue->mutex);
    }
    queue->data[queue->tail]=value;
    queue->tail=(queue->tail+1U)%QUEUE_CAPACITY;
    queue->count++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);


    return true;
}

static bool QueuePop(IntQueue *queue, int *value_out)
{
    if ((queue == NULL) || (value_out == NULL))
    {
        return false;
    }

    /*
     * TODO 2：
     * 1. 加锁
     * 2. 队列为空且生产未结束时，等待not_empty
     * 3. 队列为空且生产已经结束时，解锁并返回false
     * 4. 从head读取数据
     * 5. head循环移动，count减1
     * 6. 通知等待not_full的生产者
     * 7. 解锁并返回true
     */
    pthread_mutex_lock(&queue->mutex);
    while((queue->count==0)&&!queue->finished)
    {
        pthread_cond_wait(&queue->not_empty,&queue->mutex);
     
    }
    if((queue->count==0)&&queue->finished)
    {
            pthread_mutex_unlock(&queue->mutex);
            return false;
    }
    *value_out=queue->data[queue->head];
    queue->head=(queue->head+1U)%QUEUE_CAPACITY;
    queue->count--;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    

    return true;
}

static void QueueFinish(IntQueue *queue)
{
    pthread_mutex_lock(&queue->mutex);

    queue->finished = true;

    pthread_cond_broadcast(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
}

static void *ProducerThread(void *argument)
{
    IntQueue *queue = (IntQueue *)argument;

    for (int value = 1; value <= ITEM_COUNT; value++)
    {
        assert(QueuePush(queue, value));
    }

    QueueFinish(queue);
    return NULL;
}

static void *ConsumerThread(void *argument)
{
    ConsumerContext *context = (ConsumerContext *)argument;
    int value;

    while (QueuePop(context->queue, &value))
    {
        context->sum += value;
        context->consumed_count++;
    }

    return NULL;
}

int main(void)
{
    IntQueue queue;
    ConsumerContext context;
    pthread_t producer;
    pthread_t consumer;

    assert(QueueInit(&queue));

    context.queue = &queue;
    context.sum = 0;
    context.consumed_count = 0;

    assert(pthread_create(&consumer,
                          NULL,
                          ConsumerThread,
                          &context) == 0);

    assert(pthread_create(&producer,
                          NULL,
                          ProducerThread,
                          &queue) == 0);

    assert(pthread_join(producer, NULL) == 0);
    assert(pthread_join(consumer, NULL) == 0);

    assert(context.consumed_count == ITEM_COUNT);
    assert(context.sum == 55);

    QueueDestroy(&queue);

    printf("Day16 pthread producer-consumer tests passed\n");
    return 0;
}