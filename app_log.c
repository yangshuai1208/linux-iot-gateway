#include <stdio.h>
#include <time.h>
#include "app_log.h"

static void app_log_write(const char *level,const char *msg)
{
 FILE*fp=NULL;
 time_t now;
 struct tm*time_info;
 char time_buf[64];

 if(level==NULL||msg==NULL)
 {
  return;
 }

 fp=fopen("app_log.txt","a");
 if(fp==NULL)
 {
  perror("fopen app_log.txt failed");
  return;
 }
 
 now=time(NULL);
 time_info=localtime(&now);

 strftime(time_buf,sizeof(time_buf),"%Y-%m-%d %H:%M:%S",time_info);
 
 fprintf(fp,"[%s][%s]%s\n",time_buf,level,msg);
 fclose(fp);
}

void app_log_info(const char *msg)
{
 app_log_write("INFO",msg);
}
void app_log_warn(const char*msg)
{
 app_log_write("WARN",msg);
}
void app_log_error(const char*msg)
{
 app_log_write("ERROR",msg);
}

