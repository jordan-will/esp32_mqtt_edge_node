#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"

typedef enum{
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_GOT_IP
} wifi_state_t;

esp_err_t wifi_manager_init(const char *ssid, const char *password);
wifi_state_t wifi_manager_get_state(void);
bool wifi_manager_is_connected(void);

#endif