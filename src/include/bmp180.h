#ifndef BMP180_H
#define BMP180_H

#include "esp_err.h"

#define BMP180_SENSOR_ADDR       0x77

/* Registers */
#define BMP180_REG_CALIB_START   0xAA
#define BMP180_REG_CTRL_MEAS     0xF4
#define BMP180_REG_OUT_MSB       0xF6

/* Commands */
#define BMP180_CMD_TEMPERATURE   0x2E

#define BMP180_CMD_PRESSURE_OSS0    0x34

esp_err_t bmp180_init(void);
esp_err_t bmp180_read_temperature(float *temperature);
esp_err_t bmp180_read_pressure(float *pressure);

#endif