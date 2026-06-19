#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "include/bmp180.h"
#include "include/led_info.h"
#include "include/wifi_manager.h"
#include "include/mqtt_manager.h"

static const char *TAG = "MAIN";

//#define MQTT_BROKER_URI "mqtt://192.168.1.137:1883"
#define MQTT_BROKER_URI "mqtt://broker.hivemq.com:1883"


/* Simple wait loop for connection */
static void wait_wifi_connected(void)
{
    while (!wifi_manager_is_connected()) {
        ESP_LOGI(TAG, "Waiting Wi-Fi...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void wait_mqtt_connected(void)
{
    while (!mqtt_manager_is_connected()) {
        ESP_LOGI(TAG, "Waiting MQTT...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "System starting...");

    /* ---------------- Wi-Fi ---------------- */
    ESP_LOGI(TAG, "Initializing Wi-Fi...");
    wifi_manager_init("Wokwi-GUEST", "");

    wait_wifi_connected();
    ESP_LOGI(TAG, "Wi-Fi connected!");

    /* ---------------- MQTT ---------------- */
    ESP_LOGI(TAG, "Initializing MQTT...");
    mqtt_manager_init(MQTT_BROKER_URI);

    wait_mqtt_connected();
    ESP_LOGI(TAG, "MQTT connected!");

    /* ---------------- Sensor ---------------- */
    ESP_LOGI(TAG, "Initializing BMP180...");
    bmp180_init();

    float temperature = 0.0f;

    /* ---------------- Main loop ---------------- */
    while (1) {

        if (bmp180_read_temperature(&temperature) == ESP_OK) {

            char payload[64];
            snprintf(payload, sizeof(payload),
                     "{\"temperature\": %.2f}", temperature);

            mqtt_manager_publish(
                "jordan/willian/99/industrial/node01/temperature",
                payload,
                0,
                0
            );

            ESP_LOGI(TAG, "Published: %s", payload);
        }
        else {
            ESP_LOGE(TAG, "Sensor read failed");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/*void app_main(void)
{
    ESP_LOGI(TAG, "Starting Wi-Fi test");

    esp_err_t err = wifi_manager_init(
    "Wokwi-GUEST",  
    ""
);

    if (err != ESP_OK) {
        ESP_LOGE(
            TAG,
            "Wi-Fi initialization failed"
        );

        return;
    }

    while (1) {

        wifi_state_t state =
            wifi_manager_get_state();

        switch (state) {

        case WIFI_STATE_DISCONNECTED:

            ESP_LOGW(
                TAG,
                "Wi-Fi: DISCONNECTED"
            );

            break;

        case WIFI_STATE_CONNECTING:

            ESP_LOGI(
                TAG,
                "Wi-Fi: CONNECTING"
            );

            break;

        case WIFI_STATE_CONNECTED:

            ESP_LOGI(
                TAG,
                "Wi-Fi: CONNECTED"
            );

            break;

        case WIFI_STATE_GOT_IP:

            ESP_LOGI(
                TAG,
                "Wi-Fi: GOT_IP"
            );

            break;

        default:

            ESP_LOGW(
                TAG,
                "Wi-Fi: UNKNOWN"
            );

            break;
        }

        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );
    }
}*/

/*void app_main(void)
{
    led_info_init();

    led_info_set_wifi(
        LED_MODE_BLINK_SLOW
    );

    led_info_set_mqtt(
        LED_MODE_BLINK_FAST
    );

    while (1) {

        led_info_task();

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}*/

/*void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando teste de bancada do BMP180...");

    esp_err_t err = bmp180_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG,
                 "Falha crítica na inicialização do BMP180.");
        return;
    }

    float temperatura = 0.0f;
    float pressao = 0.0f;

    while (1) {

    
        err = bmp180_read_temperature(&temperatura);

        if (err != ESP_OK) {
            ESP_LOGE(TAG,
                     "Falha na leitura da temperatura: %s",
                     esp_err_to_name(err));

            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

       
        err = bmp180_read_pressure(&pressao);

        if (err != ESP_OK) {
            ESP_LOGE(TAG,
                     "Falha na leitura da pressão: %s",
                     esp_err_to_name(err));

            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

       
        ESP_LOGI(TAG, "====================================");
        ESP_LOGI(TAG, "Temperatura : %.2f °C", temperatura);
        ESP_LOGI(TAG, "Pressao     : %.2f hPa", pressao);
        ESP_LOGI(TAG, "====================================");

       
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}*/