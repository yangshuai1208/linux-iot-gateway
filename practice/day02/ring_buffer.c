#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BUFFER_SIZE 8

static uint8_t buffer[BUFFER_SIZE];

static size_t head = 0;
static size_t tail = 0;

bool ring_buffer_empty(void)
{
    if(head==tail)
    {
        return true;
    }
    else
    {
        return false;
    }

}
bool ring_buffer_full(void)
{
    if(((head+1U)%BUFFER_SIZE)==tail)
    {
        return true;
    }
    else
    {
        return false;
    }
  
}



bool ring_buffer_push(uint8_t data)
{
    if(ring_buffer_full())
    {
        return false;
    }
    buffer[head]=data;
    head=(head+1U)%BUFFER_SIZE;
    return true;
}
bool ring_buffer_pop(uint8_t *data)
{
    if(data==NULL)
    {
        return false;
    }
    if(ring_buffer_empty())
    {
        return false;
    }
    *data=buffer[tail];
    tail=(tail+1U)%BUFFER_SIZE;
    return true;

}



int main(void)
{
    uint8_t data = 0;

    ring_buffer_push(0x11);
    ring_buffer_push(0x22);
    ring_buffer_push(0x33);

    while (ring_buffer_pop(&data))
    {
        printf("%02X\n", data);
    }

    return 0;
}