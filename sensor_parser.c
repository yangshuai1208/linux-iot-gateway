#include <stdio.h>
#include <string.h>
#include "sensor_parser.h"

int parse_sensor_data(const char*raw,SensorData *data)
{
 int ret;
 
 if(raw==NULL||data==NULL)
 {
  return -1;
 }
 
 memset(data,0,sizeof(SensorData));
 
 ret=sscanf(raw,
           "temp=%d,humi=%d,device_id=%31s",
            &data->temp,
            &data->humi,
            data->device_id);
if(ret!=3)
{
 return -1;
}
return 0;
}
int build_json(const SensorData*data,char *json_buf,int buf_size)
{
 if(data==NULL||json_buf==NULL||buf_size<=0)
 {
  return -1;
 }
snprintf(json_buf,
         buf_size,
         "{\"temp\":%d,\"humi\"%d,\"device_id\":\"%s\"}",
         data->temp,
         data->humi,
         data->device_id);

 return 0;
}
