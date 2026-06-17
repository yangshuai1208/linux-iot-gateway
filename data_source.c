#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "data_source.h"

#if USE_MOCK_DATA==0
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
static int serial_fd=-1;

static int serial_init(int fd)
{
 struct termios options;
 
 if(tcgetattr(fd,&options)!=0)
 {
  perror("tcgetattr failed");
  return -1;
 }

cfsetispeed(&options,B115200);
cfsetospeed(&options,B115200);

#endif
