#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "mqtt_client.h"

int mqtt_client_init(struct mosquitto**mosq)
{
 int ret;
 
 if(mosq==NULL)
 {
  return -1;
 }
 mosquitto_lib_init();
 
 *mosq=mosquitto_new("gateway_module_client",true,NULL);
 if(*mosq==NULL)
 {
  printf("mosquitto_new failed\n");
  mosquitto_lib_cleanup();
  return -1;
 }
 ret=mosquitto_connect(*mosq,MQTT_HOST,MQTT_PORT,MQTT_KEEPALIVE);
 if(ret!=MOSQ_ERR_SUCCESS)
 {
  printf("mqtt connect failed:%s\n",mosquitto_strerror(ret));
  mosquitto_destroy(*mosq);
  mosquitto_lib_cleanup();
  return -1;
 }
 printf("mqtt connect success\n");
 printf("publish topic:%s\n",MQTT_TOPIC);
 
 return 0;
}
int mqtt_publish_json(struct mosquitto*mosq,const char *json_data)
{
 int ret;
 
 if(mosq==NULL||json_data==NULL)
 {
  return -1;
 }
 ret=mosquitto_publish(
     mosq,
     NULL,
     MQTT_TOPIC,
     strlen(json_data),
     json_data,
     0,
     false);

if(ret!=MOSQ_ERR_SUCCESS)
{
 printf("mqtt publish failed:%s\n",mosquitto_strerror(ret));
 return -1;
}
mosquitto_loop(mosq,1000,1);

return 0;
}
void mqtt_client_cleanup(struct mosquitto*mosq)
{
 if(mosq!=NULL)
 {
  mosquitto_disconnect(mosq);
  mosquitto_destroy(mosq); 
 }
mosquitto_lib_cleanup();
}
