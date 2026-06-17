#include <stdio.h>
#include <unistd.h>
#include <mosquitto.h>

#include "config.h"
#include "data_source.h"
#include "sensor_parser.h"
#include "log_writer.h"
#include "mqtt_client.h"
#include "app_log.h"

int main(void)
{

int i;
int ret;


char raw_buf[128];
char json_buf[128];

SensorData sensor;
struct mosquitto *mosq=NULL;

printf("gateway module start\n");
app_log_info("gateway applog start");

ret=data_source_init();
if(ret!=0)
{
 printf("data source init failed\n");
 app_log_info("data source init failed");
 return 1;
}

app_log_info("data source init success");

ret=mqtt_client_init(&mosq);
if(ret!=0)
{
 printf("mqtt client  init failed\n");
 app_log_error("mqtt client init failed");
 data_source_cleanup();
 return 1;
}
app_log_info("mqtt client init success");

for(i=0;i<LOOP_COUNT;i++)
{
  printf("-----------------\n");
  
  
  ret=data_source_read(raw_buf,sizeof(raw_buf));
  if(ret!=0)
  {
   printf("read data source failed\n");
   app_log_warn("read data source failed");
   continue;
  }
 printf("raw data:%s\n",raw_buf);

 ret=parse_sensor_data(raw_buf,&sensor);
 if(ret!=0)
 {
  printf("parse sensor data failed\n");
  app_log_warn("parse sensor data failed");
  continue;
 }

 printf("parse success\n");
 app_log_info("parse sensor data success");

 ret=build_json(&sensor,json_buf,sizeof(json_buf));
 if(ret!=0)
 {
   printf("build json failed\n");
   app_log_error("build json failed");
   continue;
 }
 printf("json:%s\n",json_buf);
 
 ret=write_log(json_buf);
 if(ret!=0)
 {
  printf("write log failed\n");
  app_log_error("write sensor log failed");
  continue;
 }
 printf("write log success\n");
 app_log_info("write sensor log success");

 ret=mqtt_publish_json(mosq,json_buf);
 if(ret!=0)
 {
  printf("mqtt publish json failed\n");
  app_log_error("mqtt publish json failed");
  continue;
 }
 printf("mqtt publish success\n");
 app_log_info("mqtt publish success");
}
mqtt_client_cleanup(mosq);
data_source_cleanup();

app_log_info("gateway applog end");
printf("gateway module end\n");

return 0;
}
  
