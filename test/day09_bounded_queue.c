#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define QUEUE_CAPACITY 4U
#define TEST_ITEM_COUNT 1000U

typedef struct
{
    uint32_t sequence;
    uint16_t temp_x10;
    uint16_t humi_x10;
} SensorMessage;

typedef enum
{
    QUEUE_OK = 0,
    QUEUE_CLOSED,
    QUEUE_ERROR
} QueueResult;

typedef struct
{
    SensorMessage items[QUEUE_CAPACITY];

    size_t head;     /* 下一次读取位置 */
    size_t tail;     /* 下一次写入位置 */
    size_t count;

    bool closed;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} BoundedQueue;

static bool queue_init(BoundedQueue *queue)
{
    /*
     * TODO：
     * 1. 检查queue。
     * 2. 整个结构体清零。
     * 3. 初始化mutex。
     * 4. 初始化not_empty。
     * 5. 初始化not_full。
     * 6. 某一步失败时释放此前已初始化的资源。
     */
    if(queue==NULL)
    {
        return false;
    }
   memset(queue,0,sizeof(*queue));
   
  if(pthread_mutex_init(&queue->mutex,NULL)!=0)
  {
    return false;
  }
  if(pthread_cond_init(&queue->not_empty,NULL)!=0)
  {
    pthread_mutex_destroy(&queue->mutex);
    return false;
  }
  if(pthread_cond_init(&queue->not_full,NULL)!=0)
  {
    pthread_cond_destroy(&queue->not_empty);
    pthread_mutex_destroy(&queue->mutex);
    return false;
  }
  return true;
}

static void queue_destroy(BoundedQueue *queue)
{
    /*
     * TODO：
     * 销毁两个条件变量和互斥量。
     * 调用本函数前必须保证线程已经退出。
     */

     if(queue==NULL)
     {
        return;
     }

     pthread_cond_destroy(&queue->not_empty);
     pthread_cond_destroy(&queue->not_full);
     pthread_mutex_destroy(&queue->mutex);
} 

static QueueResult queue_push(
    BoundedQueue *queue,
    const SensorMessage *message)
{
    /*
     * TODO：
     * 1. 检查参数。
     * 2. 加锁。
     * 3. 队列满且未关闭时，等待not_full。
     * 4. 被唤醒后必须重新检查条件。
     * 5. 如果队列关闭，解锁并返回QUEUE_CLOSED。
     * 6. 在tail处保存结构体副本。
     * 7. tail循环前进，count增加。
     * 8. 通知not_empty。
     * 9. 解锁并返回QUEUE_OK。
     */
    if(queue==NULL||message==NULL)
    {
        return QUEUE_ERROR;
    }
    if(pthread_mutex_lock(&queue->mutex)!=0)
    {
        return QUEUE_ERROR;
    }

   while(queue->count==QUEUE_CAPACITY&&!queue->closed)
   {
    if(pthread_cond_wait(&queue->not_full,&queue->mutex)!=0)
    {
        pthread_mutex_unlock(&queue->mutex);
        return QUEUE_ERROR;
    }
    }
    if(queue->closed)
    {
        pthread_mutex_unlock(&queue->mutex);
        return QUEUE_CLOSED;
    }
    queue->items[queue->tail]=*message;
    queue->tail=(queue->tail+1U)%QUEUE_CAPACITY;
    ++queue->count;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);

    return QUEUE_OK;
   }


static QueueResult queue_pop(
    BoundedQueue *queue,
    SensorMessage *output)
{
    /*
     * TODO：
     * 1. 检查参数。
     * 2. 加锁。
     * 3. 队列空且未关闭时，等待not_empty。
     * 4. 被唤醒后必须重新检查条件。
     * 5. 如果队列为空且已经关闭，返回QUEUE_CLOSED。
     * 6. 从head读取结构体副本。
     * 7. head循环前进，count减少。
     * 8. 通知not_full。
     * 9. 解锁并返回QUEUE_OK。
     */
      if (queue == NULL || output == NULL)
    {
        return QUEUE_ERROR;
    }

    if (pthread_mutex_lock(&queue->mutex) != 0)
    {
        return QUEUE_ERROR;
    }

    while (queue->count == 0U &&
           !queue->closed)
    {
        if (pthread_cond_wait(
                &queue->not_empty,
                &queue->mutex) != 0)
        {
            pthread_mutex_unlock(&queue->mutex);
            return QUEUE_ERROR;
        }
    }

 
   
    if (queue->count == 0U && queue->closed)
    {
        pthread_mutex_unlock(&queue->mutex);
        return QUEUE_CLOSED;
    }

    *output = queue->items[queue->head];

    queue->head =
        (queue->head + 1U) % QUEUE_CAPACITY;

    --queue->count;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);

    return QUEUE_OK;
}

static bool queue_close(BoundedQueue *queue)
{
    /*
     * TODO：
     * 1. 检查参数。
     * 2. 加锁。
     * 3. closed设为true。
     * 4. broadcast唤醒全部生产者和消费者。
     * 5. 解锁。
     */

     if(queue==NULL)
     {
        return false;
     }

     if(pthread_mutex_lock(&queue->mutex)!=0)
     {
        return false;
     }
     queue->closed=true;
     int empty_result=pthread_cond_broadcast(&queue->not_empty);
     int full_result=pthread_cond_broadcast(&queue->not_full);
     int unlock_result=pthread_mutex_unlock(&queue->mutex);
       return empty_result == 0 &&
           full_result == 0 &&
           unlock_result == 0;
}
typedef struct
{
    BoundedQueue *queue;
    bool failed;
} ProducerContext;

typedef struct
{
    BoundedQueue *queue;
    bool failed;
    uint32_t received_count;
} ConsumerContext;

static void *producer_thread(void *argument)
{
    ProducerContext *context =
        (ProducerContext *)argument;

    for (uint32_t sequence = 1U;
         sequence <= TEST_ITEM_COUNT;
         ++sequence)
    {
        SensorMessage message =
        {
            sequence,
            (uint16_t)(250U + sequence % 10U),
            (uint16_t)(600U + sequence % 10U)
        };

        if (queue_push(
                context->queue,
                &message) != QUEUE_OK)
        {
            context->failed = true;
            (void)queue_close(context->queue);
            return NULL;
        }
    }

    /*
     * 生产结束，关闭队列并唤醒消费者。
     */
    if (!queue_close(context->queue))
    {
        context->failed = true;
    }

    return NULL;
}

static void *consumer_thread(void *argument)
{
    ConsumerContext *context =
        (ConsumerContext *)argument;

    uint32_t expected_sequence = 1U;

    for (;;)
    {
        SensorMessage message = {0};

        QueueResult result =
            queue_pop(context->queue, &message);

        if (result == QUEUE_CLOSED)
        {
            break;
        }

        if (result != QUEUE_OK)
        {
            context->failed = true;
            (void)queue_close(context->queue);
            return NULL;
        }

        /*
         * 检查FIFO顺序。
         */
        if (message.sequence != expected_sequence)
        {
            context->failed = true;
            (void)queue_close(context->queue);
            return NULL;
        }

        ++expected_sequence;
    }

    context->received_count =
        expected_sequence - 1U;

    return NULL;
}

int main(void)
{
    BoundedQueue queue;

    assert(queue_init(&queue));

    ProducerContext producer_context =
    {
        &queue,
        false
    };

    ConsumerContext consumer_context =
    {
        &queue,
        false,
        0U
    };

    pthread_t producer;
    pthread_t consumer;

    /*
     * 先启动消费者，使其在空队列上等待。
     */
    assert(pthread_create(
        &consumer,
        NULL,
        consumer_thread,
        &consumer_context) == 0);

    assert(pthread_create(
        &producer,
        NULL,
        producer_thread,
        &producer_context) == 0);

    assert(pthread_join(producer, NULL) == 0);
    assert(pthread_join(consumer, NULL) == 0);

    assert(!producer_context.failed);
    assert(!consumer_context.failed);

    assert(consumer_context.received_count ==
           TEST_ITEM_COUNT);

    assert(queue.count == 0U);
    assert(queue.closed);

    queue_destroy(&queue);

    printf("bounded queue tests passed\n");
    return 0;
}