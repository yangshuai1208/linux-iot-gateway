#include <stdio.h>
#include <time.h>
#include "log_writer.h"

int write_log(const char*json_data)
{
  FILE*fp=NULL;
  time_t now;
  struct tm *time_info;
  char time_buf[64];
  
  if(json_data==NULL)
  {
  return -1;
  }
fp=fopen("sensor_log.txt","a");
if(fp==NULL)
{
 perror("fopen sensor_log.txt failed");
 return -1;
}
now=time(NULL);
time_info=localtime(&now);

strftime(time_buf,sizeof(time_buf),"%Y-%m-%d %H:%M:%S",time_info);

fprintf(fp,"[%s]%s\n",time_buf,json_data);

fclose(fp);

return 0;
}
