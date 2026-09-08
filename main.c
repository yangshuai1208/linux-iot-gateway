#include <stdio.h>
#include <signal.h>
#include <mosquitto.h>

#include "config.h"
#include "data_source.h"
#include "sensor_parser.h"
#include "log_writer.h"
#include "mqtt_client.h"
#include "app_log.h"

static volatile sig_atomic_t g_running = 1;

static void signal_handler(int signo)
{
    (void)signo;
    g_running = 0;
}

int main(void)
{
    int ret;

    char raw_buf[128];
    char json_buf[128];

    SensorData sensor;
    struct mosquitto *mosq = NULL;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("gateway start\n");
    app_log_info("gateway start");

    ret = data_source_init();
    if (ret != 0)
    {
        printf("data source init failed\n");
        app_log_error("data source init failed");
        return 1;
    }

    app_log_info("data source init success");

    ret = mqtt_client_init(&mosq);
    if (ret != 0)
    {
        printf("mqtt client init failed\n");
        app_log_error("mqtt client init failed");

        data_source_cleanup();
        return 1;
    }

    app_log_info("mqtt client init success");

    while (g_running)
    {
        printf("-----------------\n");

        ret = data_source_read(raw_buf, sizeof(raw_buf));
        if (ret != 0)
        {
            if (!g_running)
            {
                break;
            }

            printf("read data source failed\n");
            app_log_warn("read data source failed");
            continue;
        }

        printf("raw data: %s\n", raw_buf);

        ret = parse_sensor_data(raw_buf, &sensor);
        if (ret != 0)
        {
            printf("parse sensor data failed\n");
            app_log_warn("parse sensor data failed");
            continue;
        }

        printf("parse success\n");
        app_log_info("parse sensor data success");

        ret = build_json(&sensor, json_buf, sizeof(json_buf));
        if (ret != 0)
        {
            printf("build json failed\n");
            app_log_error("build json failed");
            continue;
        }

        printf("json: %s\n", json_buf);

        ret = write_log(json_buf);
        if (ret != 0)
        {
            printf("write log failed\n");
            app_log_error("write sensor log failed");
            continue;
        }

        app_log_info("write sensor log success");

        ret = mqtt_publish_json(mosq, json_buf);
        if (ret != 0)
        {
            printf("mqtt publish failed\n");
            app_log_error("mqtt publish failed");
            continue;
        }

        printf("mqtt publish success\n");
        app_log_info("mqtt publish success");
    }

    printf("\ngateway stopping...\n");
    app_log_info("gateway stopping");

    mqtt_client_cleanup(mosq);
    data_source_cleanup();

    app_log_info("gateway stopped");
    printf("gateway stopped\n");

    return 0;
}