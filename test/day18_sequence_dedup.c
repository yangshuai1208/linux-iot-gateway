#include <stdio.h>
#include <assert.h>

typedef struct
{
    unsigned int seq;
    char cmd[32];
} CommandRequest;

int process_command(const CommandRequest *req)
{
    static unsigned int last_seq=0;
    if(req==NULL)
    {
        return -1;
    }
    if(req->seq==last_seq)
    {
        return 0;
    }
    else
    {
        last_seq=req->seq;
        return 1;
    }
    

}

int main()
{
    CommandRequest req1 = {1, "OPEN"};
    CommandRequest req2 = {2, "GRAB"};
    CommandRequest req3 = {2, "GRAB"};
    CommandRequest req4 = {3, "STOP"};  

    assert(process_command(&req1) == 1);
    assert(process_command(&req2) == 1);
    assert(process_command(&req3) == 0);
    assert(process_command(&req4) == 1);

    printf("all tests passed\n");

    return 0;

}