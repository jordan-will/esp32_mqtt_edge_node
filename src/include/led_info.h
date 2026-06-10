#ifndef LED_INFO_H
#define LED_INFO_H

#include "esp_err.h"

typedef enum{
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_BLINK_SLOW,
    LED_MODE_BLINK_FAST
} led_mode_t;

/**
 * @brief Initialize the LED GPIO pin.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t led_info_init(void);

/**
 * @brief set wi-fi led mode
 * @param mode desired led mode
 */
void led_info_set_wifi(led_mode_t mode);

/**
 * @brief set mqtt led mod
 * @param mode desired led mode
 */
void led_info_set_mqtt(led_mode_t mode);

/**
 * @brief update led fsm
 * 
 * must be called periodically
 */
void led_info_task(void);
#endif