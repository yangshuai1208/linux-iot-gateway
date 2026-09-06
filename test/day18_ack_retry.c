#include <stdio.h>
#include <assert.h>

typedef struct
{
    unsigned int seq;
    char cmd[32];

    unsigned int retry_count;
    unsigned int max_retry;

    int ack_received;
} PendingRequest;

int should_retry(const PendingRequest *req)
{
    if (req == NULL)
    {
        return -1;
    }

    if (req->ack_received)
    {
        return 0;
    }

    if (req->retry_count < req->max_retry)
    {
        return 1;
    }

    return 0;
}
void simulate_send(PendingRequest *req)
{
    if(req==NULL)
    {
        return ;

    }   
    while(should_retry(req)==1)
    {
        printf("SEND seq=%u cmd=%s retry=%u\n",req->seq,req->cmd,req->retry_count);
        req->retry_count++;

    }
    if(req->ack_received==1)
    {
        printf("SUCCESS");
    }
    else{
     printf("FAILED seq=%u after %u retries\n",
       req->seq,
       req->retry_count);
    }
}
void simulate_send_with_ack(PendingRequest *req)
{
    if(req==NULL)
    {
        return ;
    }
    while(should_retry(req)==1)
    {
            printf("SEND seq=%u cmd=%s retry=%u\n",req->seq,req->cmd,req->retry_count);
            req->retry_count++;
            

        if (req->retry_count == 2)
        {
            req->ack_received = 1;

            printf("ACK RECEIVED seq=%u\n",
                   req->seq);
        }
    }
    if(req->ack_received==1)
    {
        printf("SUCCESS seq:%u",req->seq);
    }
        else
    {
        printf("FAILED seq=%u after %u retries\n",
               req->seq,
               req->retry_count);
    }
}

int main()
{
PendingRequest req1 = {15, "HAND_GRAB", 0, 3, 0};
PendingRequest req2 = {15, "HAND_GRAB", 2, 3, 0};
PendingRequest req3 = {15, "HAND_GRAB", 3, 3, 0};
PendingRequest req4 = {15, "HAND_GRAB", 1, 3, 1};
PendingRequest retry_test =
    {15, "HAND_GRAB", 0, 3, 0};
PendingRequest ack_test =
    {20, "HAND_OPEN", 0, 3, 0};


    assert(should_retry(&req1) == 1);
    assert(should_retry(&req2) == 1);
    assert(should_retry(&req3) == 0);
    assert(should_retry(&req4) == 0);
    assert(should_retry(NULL) == -1);
    simulate_send(&retry_test);
    simulate_send_with_ack(&ack_test);

     printf("all retry tests passed\n");
return 0;
}