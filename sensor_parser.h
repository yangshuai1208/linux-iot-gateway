#ifndef SENSOR_PARSER_H
#define SENSOR_PARSER_H

typedef struct
{
 int temp;
 int humi;
 char device_id[32];
}SensorData;

int parse_sensor_data(const char *raw,SensorData *data);
int build_json(const SensorData*data,char *json_buf,int buf_size);

#endif
