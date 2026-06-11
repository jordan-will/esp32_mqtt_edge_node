#include "../include/wifi_manager.h"

#include <string.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h" 

static const char *TAG = "WIFI_MANAGER";

static wifi_state_t wifi_state = WIFI_STATE_DISCONNECTED;

static char wifi_ssid[32];
static char wifi_password[64];

//wifi event handler
static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
){
    if(event_base == WIFI_EVENT){
        switch(event_id)
        {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Wi-FI started");
                wifi_state = WIFI_STATE_CONNECTING;
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to AP");
                wifi_state = WIFI_STATE_CONNECTED;
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "Disconnectedd");
                wifi_state = WIFI_STATE_DISCONNECTED;
                esp_wifi_connect();
            default:
                break;
        }
    }//endif wifi event

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "GOT IP: " IPSTR, 
            IP2STR(&event->ip_info.ip)
        );

        wifi_state = WIFI_STATE_CONNECTED;
    }
}

esp_err_t wifi_manager_init(
    const char *ssid,
    const char *password
){
    esp_err_t err;

    //validate arguments
    if(ssid == NULL || password == NULL) return ESP_ERR_INVALID_ARG;

    //store credentials locally
    strncpy(
        wifi_ssid,
        ssid,
        sizeof(wifi_ssid) -1
    );

    wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';

    strncpy(
        wifi_password,
        password,
        sizeof(wifi_password) -1
    );

    wifi_password[sizeof(wifi_password) -1] = '\0';

    //initialize nvs
    err = nvs_flash_init();

    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if(err != ESP_OK){
        ESP_LOGE(TAG, "NVS initalization failled");
        return err;
    }

    //initialize tcp/ip
    err = esp_netif_init();

    if(err != ESP_OK){
        ESP_LOGE(TAG, "esp_netif_init failed");
        return err;
    }

    //create default event loop
    err = esp_event_loop_create_default();

    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "Event loop creation failed");
        return err;
    }

    //create default sta interface
    esp_netif_create_default_wifi_sta();

    //initialize wi-fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&cfg);

    if(err != ESP_OK){
        ESP_LOGE(TAG, "esp_wifi_init failed");
        return err;
    }

    err = esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        NULL
    );

    if(err != ESP_OK){
        ESP_LOGE(TAG, "Wi-Fi handler registration failed");
        return err;
    }

    //register ip event
    err = esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL,
        NULL
    );

    if(err != ESP_OK){
        ESP_LOGE(TAG, "IP handler registration failed");
        return err;
    }

    //configure stattion mode
    wifi_config_t wifi_config = {0};

    strncpy(
        (char *)wifi_config.sta.ssid,
        wifi_ssid,
        sizeof(wifi_config.sta.ssid) - 1
    );

    strncpy(
        (char *)wifi_config.sta.password,
        wifi_password,
        sizeof(wifi_config.sta.password) -1
    );

    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    //wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    //set startion mode
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set STA mode");
        return err;
    }

    //apply wi-fi configuration
    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to set Wi-Fi config");
        return err;
    }

    err = esp_wifi_start();
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to start Wi-fi");
        return err;
    }

    ESP_LOGI(TAG, "Wi-Fi manager initializedd");
    return ESP_OK;
}

wifi_state_t wifi_manager_get_state(void){
    return wifi_state;
}

bool wifi_manager_is_connected(void){
    return (wifi_state == WIFI_STATE_GOT_IP);
}