#include "../include/mqtt_manager.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <string.h>

static const char *TAG = "MQTT_MANAGER";

//mqtt handler
static esp_mqtt_client_handle_t client =  NULL;

//internal state
static mqtt_state_t state = MQTT_STATE_DISCONNECTED;

//broker uri storage
static char broker_uri[128];

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
);

esp_err_t mqtt_manager_init(const char *uri){
    if(!uri){
        ESP_LOGI(TAG, "Fail: UIR not setted");
        return ESP_ERR_INVALID_ARG;
    }

    strncpy(broker_uri, uri, sizeof(broker_uri) - 1);

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = broker_uri
    };

    client = esp_mqtt_client_init(&cfg);
    if(!client){
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL
    );
    ESP_LOGI(TAG, "MQTT SERVER = %s", broker_uri);
    state = MQTT_STATE_CONNECTING;
    
    return esp_mqtt_client_start(client);
}

esp_err_t mqtt_manager_publish(
    const char *topic,
    const char *payload,
    int qos,
    int retain
){
    if(state != MQTT_STATE_CONNECTED && state != MQTT_STATE_SUBSCRIBED){
        ESP_LOGI(TAG, "Fail: state not subscribed");
        return ESP_ERR_INVALID_STATE;
    }

    if(!topic || !payload){
        ESP_LOGI(TAG, "Fail: topic or payload not setted");
        return ESP_ERR_INVALID_ARG;
    }

    int msg_id = esp_mqtt_client_publish(
        client,
        topic,
        payload,
        0,
        qos,
        retain
    );

    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t mqtt_manager_subscribe(const char *topic, int qos){
    
    if(!topic){
        ESP_LOGI(TAG, "Fail: topic on subscribed not setted");
        return ESP_ERR_INVALID_ARG;
    }

    if(state != MQTT_STATE_CONNECTED && state != MQTT_STATE_SUBSCRIBED){
        ESP_LOGE(TAG, "Failed: state not connected or subscribed on subscribe");
        return ESP_ERR_INVALID_STATE;
    }

    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);

    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

mqtt_state_t mqtt_manager_get_state(void){
    return state;
}

bool mqtt_manager_is_connected(void){
    return state == MQTT_STATE_CONNECTED || state == MQTT_STATE_SUBSCRIBED;
}

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
){
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    switch(event_id){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected to Broker");
            state = MQTT_STATE_CONNECTED;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            state = MQTT_STATE_DISCONNECTED;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT subscribed");
            state = MQTT_STATE_SUBSCRIBED;
            break;
        case  MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT message published");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Data: %.*s", event->data_len, event->data);
            break;
        default:
            break;

    }
}