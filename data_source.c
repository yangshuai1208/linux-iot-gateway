#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "data_source.h"

#if USE_MOCK_DATA == 0
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#endif

#if USE_MOCK_DATA==1

static const char*mock_data_list[]=
{
 "temp=25,humi=60,device_id=stm32_01",
 "temp=26,humi=61,device_id=stm32_01",
 "bad data_from_stm32",
 "temp=28,humi=63,device_id=stm32_01",
 "temp=29,humi=64,device_id=stm32_01"
};
static int mock_index=0;

int data_source_init(void)
{
 printf("data source:mock mode\n");
 return 0;
}
int data_source_read(char*buf,int buf_size)
{
 int count;
 
 if(buf==NULL||buf_size<=0)
 {
  return -1;
 }
count=sizeof(mock_data_list)/sizeof(mock_data_list[0]);
 
 snprintf(buf,buf_size,"%s",mock_data_list[mock_index]);
 
 mock_index++;
 if(mock_index>=count)
 {
   mock_index=0;
 }
 sleep(2);
 return 0;
}
void data_source_cleanup(void)
{
 printf("data source cleanup\n");
}
#else

static int serial_fd = -1;

static int serial_configure(int fd)
{
    struct termios options;

    if (tcgetattr(fd, &options) != 0)
    {
        perror("tcgetattr failed");
        return -1;
    }

    if (cfsetispeed(&options, B115200) != 0 ||
        cfsetospeed(&options, B115200) != 0)
    {
        perror("set baud rate failed");
        return -1;
    }

    
    options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    options.c_cflag |= CS8;

    
    options.c_cflag |= CREAD | CLOCAL;

#ifdef CRTSCTS
    /* 关闭硬件流控 */
    options.c_cflag &= ~CRTSCTS;
#endif


    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(IGNBRK |
                         BRKINT |
                         PARMRK |
                         ISTRIP |
                         INLCR |
                         IGNCR |
                         ICRNL);

   
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  
    options.c_oflag &= ~OPOST;

  
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;

    if (tcflush(fd, TCIFLUSH) != 0)
    {
        perror("tcflush failed");
        return -1;
    }

    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("tcsetattr failed");
        return -1;
    }

    return 0;
}

int data_source_init(void)
{
    serial_fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY);

    if (serial_fd < 0)
    {
        perror("open serial device failed");
        return -1;
    }

    if (serial_configure(serial_fd) != 0)
    {
        close(serial_fd);
        serial_fd = -1;
        return -1;
    }

    printf("data source: serial mode, device=%s\n", SERIAL_DEV);
    return 0;
}

int data_source_read(char *buf, int buf_size)
{
    int total = 0;
    char ch;

    if (buf == NULL || buf_size <= 1 || serial_fd < 0)
    {
        return -1;
    }

    buf[0] = '\0';

    while (total < buf_size - 1)
    {
        ssize_t received = read(serial_fd, &ch, 1U);

        if (received > 0)
        {
            if (ch == '\r' || ch == '\n')
            {
                if (total > 0)
                {
                    break;
                }

                /* 忽略一帧开始前残留的换行符 */
                continue;
            }

            buf[total] = ch;
            ++total;
        }
        else if (received == 0)
        {
            /* 串口读取超时 */
            break;
        }
        else
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("read serial device failed");
            return -1;
        }
    }

    buf[total] = '\0';

    if (total == 0)
    {
        return -1;
    }

    return 0;
}

void data_source_cleanup(void)
{
    if (serial_fd >= 0)
    {
        if (close(serial_fd) != 0)
        {
            perror("close serial device failed");
        }

        serial_fd = -1;
    }

    printf("data source cleanup\n");
}

#endif