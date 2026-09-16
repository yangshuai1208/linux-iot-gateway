#include <stdint.h>
#include <stdio.h>  


#define HEAD 0xAA
#define TAIL 0x55
#define MAX_PAYLOAD 32


typedef enum
{
    WAIT_HEAD,
    WAIT_LEN,
    WAIT_PAYLOAD,
    WAIT_CHECKSUM,
    WAIT_TAIL
} ParserState;

static ParserState state=WAIT_HEAD;

static uint8_t payload[MAX_PAYLOAD];
static uint8_t payload_len=0;
static uint8_t payload_index=0;
static uint8_t checksum=0;


void parser_feed(uint8_t byte)
{
  switch(state)
  {
    case WAIT_HEAD:
    {
        if(byte==HEAD)
        {
            state=WAIT_LEN;
        }
        break;
    }
case WAIT_LEN:
{
    if (byte <= MAX_PAYLOAD)
    {
        payload_len = byte;
        payload_index = 0;
        checksum = 0;

        if (payload_len == 0)
        {
            state = WAIT_CHECKSUM;
        }
        else
        {
            state = WAIT_PAYLOAD;
        }
    }
    else
    {
        printf("Length error\n");
        state = WAIT_HEAD;
    }

    break;
}
    case WAIT_PAYLOAD:
    {
        payload[payload_index++]=byte;

        checksum=(uint8_t)(checksum+byte);

        if(payload_index>=payload_len)
        {
            state=WAIT_CHECKSUM;
        }
        break;
    }
    case WAIT_CHECKSUM:
    {
        if(byte==checksum)
        {
            state=WAIT_TAIL;
        }
        else
        {
            printf("Checksum error\n");
            state=WAIT_HEAD;
        }
        break;
    }
    case WAIT_TAIL:
    {
        if(byte==TAIL)
        {
            printf("Frame OK len=%u,payload:",(unsigned int)payload_len);

            for(uint8_t i=0;i<payload_len;i++)
            {
                printf("%02X",payload[i]);
            }
            printf("\n");
        }
        else 
        {
            printf("Tail error\n");
        }
        state=WAIT_HEAD;
        break;
    }
    default:
    {
        state=WAIT_HEAD;
        break;
    }
  }


}
int main(void)
{
    uint8_t test[] =
    {
        0xAA,
        0x03,
        0x01,
        0x02,
        0x03,
        0x06,
        0x55
    };

    for(uint32_t i=0;i<sizeof(test)/sizeof(test[0]);i++)
    {
        parser_feed(test[i]);
    }

    return 0;
}