#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "esp_err.h" 
#include <stdbool.h>

typedef enum {
    MQTT_STATE_DISCONNECTED,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_SUBSCRIBED
} mqtt_state_t;

#define IP_ADRESS "192.168.1.137"

esp_err_t mqtt_manager_init(const char *broker_url);

esp_err_t mqtt_manager_publish(
    const char *topic,
    const char *payload,
    int qos,
    int retain 
);

esp_err_t mqtt_manager_subscribe(const char *topic, int qos);

mqtt_state_t mqtt_manager_get_state(void);

bool mqtt_manager_is_connected(void);

#endif