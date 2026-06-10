#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "include/bmp180.h"

static const char *TAG = "MAIN_TEST";

#include "include/led_info.h"

void app_main(void)
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
}

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