#include <stdio.h>
#include <string.h>
#include "sensor_parser.h"
#include <stddef.h>

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
int build_json(const SensorData *data,
               char *json_buf,
               size_t buf_size)
{
    int written;

    if (data == NULL || json_buf == NULL || buf_size == 0U)
    {
        return -1;
    }

    written = snprintf(
        json_buf,
        buf_size,
        "{\"temp\":%d,\"humi\":%d,\"device_id\":\"%s\"}",
        data->temp,
        data->humi,
        data->device_id
    );

    if (written < 0 || (size_t)written >= buf_size)
    {
        json_buf[0] = '\0';
        return -1;
    }

    return 0;
}
