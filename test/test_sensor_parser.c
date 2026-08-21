#include "sensor_parser.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    SensorData data = {
        .temp = 25,
        .humi = 60,
        .device_id = "node01"
    };

    char json[128];
    char small_buffer[8];

    assert(build_json(&data, json, sizeof(json)) == 0);

    assert(strcmp(
        json,
        "{\"temp\":25,\"humi\":60,\"device_id\":\"node01\"}"
    ) == 0);

    assert(build_json(NULL, json, sizeof(json)) == -1);
    assert(build_json(&data, NULL, sizeof(json)) == -1);
    assert(build_json(&data, small_buffer, sizeof(small_buffer)) == -1);

    printf("sensor parser tests passed\n");

    SensorData parsed;

assert(parse_sensor_data(
    "temp=25,humi=60,device_id=node01",
    &parsed
) == 0);

assert(parsed.temp == 25);
assert(parsed.humi == 60);
assert(strcmp(parsed.device_id, "node01") == 0);

assert(parse_sensor_data("invalid data", &parsed) == -1);
assert(parse_sensor_data(NULL, &parsed) == -1);
assert(parse_sensor_data("temp=25,humi=60", &parsed) == -1);
    return 0;
}