#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <mosquitto.h>

#define MQTT_HOST "localhost"
#define MQTT_PORT 1883
#define MQTT_KEEPALIVE 60
#define MQTT_TOPIC "gateway/stm32_01/sensor"

int mqtt_client_init(struct mosquitto**mosq);
int mqtt_publish_json(struct mosquitto*mosq,const char*json_data);
void mqtt_client_cleanup(struct mosquitto *mosq);

#endif
