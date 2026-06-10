#include "../include/led_info.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#define WIFI_LED_GPIO GPIO_NUM_2
#define MQTT_LED_GPIO GPIO_NUM_4
#define BLINK_SLOW_MS 500 
#define BLINK_FAST_MS 100

static const char *TAG = "LED_INFO";

typedef struct {
    gpio_num_t pin;
    led_mode_t mode;
    bool output_state;
    TickType_t last_toggle;
    TickType_t interval;
} led_t;

static led_t wifi_led;
static led_t mqtt_led;

static void led_update(led_t *led){
    TickType_t now = xTaskGetTickCount();

    switch(led->mode){
        case LED_MODE_OFF:
            gpio_set_level(led->pin, 0);
            led->output_state = false;
            break;
        case LED_MODE_ON:
            gpio_set_level(led->pin, 1);
            led->output_state = true;
            break;
        case LED_MODE_BLINK_SLOW:
        case LED_MODE_BLINK_FAST:
            if((now - led->last_toggle) >= led->interval){
                led->output_state = !led->output_state;
                gpio_set_level(led->pin, led->output_state);
                led->last_toggle = now;
            }
            break;
        default:
            break;
    }
}

static void led_set_mode(led_t *led, led_mode_t mode){
    led->mode = mode;
    led->last_toggle = xTaskGetTickCount();

    switch(mode){
        case LED_MODE_BLINK_SLOW:
            led->interval = pdMS_TO_TICKS(BLINK_SLOW_MS);
            break;
        case LED_MODE_BLINK_FAST:
            led->interval = pdMS_TO_TICKS(BLINK_FAST_MS);
            break;
        default:
            break;
    }
}

esp_err_t led_info_init(void){

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << WIFI_LED_GPIO) | (1ULL << MQTT_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };   

    esp_err_t err = gpio_config(&io_conf);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "GPIO initliazed failed");
        return err;
    }
    
    wifi_led.pin = WIFI_LED_GPIO;
    mqtt_led.pin = MQTT_LED_GPIO;

    wifi_led.output_state = false;
    mqtt_led.output_state = false;

    led_set_mode(&wifi_led, LED_MODE_OFF);
    led_set_mode(&mqtt_led, LED_MODE_OFF);

    ESP_LOGI(TAG, "LED module, initialized successfully");

    return ESP_OK;
}

void led_info_set_wifi(led_mode_t mode){
    led_set_mode(&wifi_led, mode);
}

void led_info_set_mqtt(led_mode_t mode){
    led_set_mode(&mqtt_led, mode);
}

void led_info_task(void){
    led_update(&wifi_led);
    led_update(&mqtt_led);
}