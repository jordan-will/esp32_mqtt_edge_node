#include "../include/bmp180.h"
#include "../include/i2c_bus.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BMP180";

/* BMP180 device handle */
static i2c_master_dev_handle_t bmp180_dev_handle = NULL;

/* Factory calibration coefficients */
typedef struct {
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;

    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;

    int16_t B1;
    int16_t B2;

    int16_t MB;
    int16_t MC;
    int16_t MD;
} bmp180_calibration_t;

static bmp180_calibration_t calib;

/* Read factory calibration data */
static esp_err_t bmp180_read_calibration(void)
{
    uint8_t reg = BMP180_REG_CALIB_START;
    uint8_t data[22];

    esp_err_t err = i2c_master_transmit_receive(
        bmp180_dev_handle,
        &reg,
        1,
        data,
        sizeof(data),
        -1
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read calibration data");
        return err;
    }

    calib.AC1 = (data[0]  << 8) | data[1];
    calib.AC2 = (data[2]  << 8) | data[3];
    calib.AC3 = (data[4]  << 8) | data[5];

    calib.AC4 = (data[6]  << 8) | data[7];
    calib.AC5 = (data[8]  << 8) | data[9];
    calib.AC6 = (data[10] << 8) | data[11];

    calib.B1  = (data[12] << 8) | data[13];
    calib.B2  = (data[14] << 8) | data[15];

    calib.MB  = (data[16] << 8) | data[17];
    calib.MC  = (data[18] << 8) | data[19];
    calib.MD  = (data[20] << 8) | data[21];

    ESP_LOGI(TAG, "Calibration data loaded");

    return ESP_OK;
}

/* Compute B5 compensation term from raw temperature */
static esp_err_t bmp180_compute_b5(int32_t *B5)
{
    uint8_t cmd[2] = {
        BMP180_REG_CTRL_MEAS,
        BMP180_CMD_TEMPERATURE
    };

    esp_err_t err = i2c_master_transmit(
        bmp180_dev_handle,
        cmd,
        sizeof(cmd),
        -1
    );

    if (err != ESP_OK) {
        return err;
    }

    /* Wait for temperature conversion */
    vTaskDelay(pdMS_TO_TICKS(5));

    uint8_t reg = BMP180_REG_OUT_MSB;
    uint8_t data[2];

    err = i2c_master_transmit_receive(
        bmp180_dev_handle,
        &reg,
        1,
        data,
        sizeof(data),
        -1
    );

    if (err != ESP_OK) {
        return err;
    }

    int32_t UT =
        ((int32_t)data[0] << 8) |
        (int32_t)data[1];

    int32_t X1 =
        ((UT - calib.AC6) * calib.AC5) >> 15;

    int32_t X2 =
        ((int32_t)calib.MC << 11) /
        (X1 + calib.MD);

    *B5 = X1 + X2;

    return ESP_OK;
}

esp_err_t bmp180_init(void)
{
    esp_err_t err;

    err = i2c_bus_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        return err;
    }

    err = i2c_bus_add_device(
        BMP180_SENSOR_ADDR,
        &bmp180_dev_handle
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register BMP180 device");
        return err;
    }

    err = bmp180_read_calibration();

    if (err != ESP_OK) {
        return err;
    }

    ESP_LOGI(TAG, "BMP180 initialized");

    return ESP_OK;
}

esp_err_t bmp180_read_temperature(float *temperature)
{
    if (bmp180_dev_handle == NULL) {
        ESP_LOGE(TAG, "Driver not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (temperature == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Start temperature conversion */
    uint8_t cmd[2] = {
        BMP180_REG_CTRL_MEAS,
        BMP180_CMD_TEMPERATURE
    };

    esp_err_t err = i2c_master_transmit(
        bmp180_dev_handle,
        cmd,
        sizeof(cmd),
        -1
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start temperature conversion");
        return err;
    }

    /* Wait for conversion completion */
    vTaskDelay(pdMS_TO_TICKS(5));

    /* Read uncompensated temperature */
    uint8_t reg = BMP180_REG_OUT_MSB;
    uint8_t data[2];

    err = i2c_master_transmit_receive(
        bmp180_dev_handle,
        &reg,
        1,
        data,
        sizeof(data),
        -1
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature");
        return err;
    }

    int32_t UT =
        ((int32_t)data[0] << 8) |
        (int32_t)data[1];

    /* Bosch compensation algorithm */
    int32_t X1 =
        ((UT - calib.AC6) * calib.AC5) >> 15;

    int32_t X2 =
        ((int32_t)calib.MC << 11) /
        (X1 + calib.MD);

    int32_t B5 = X1 + X2;

    int32_t T =
        (B5 + 8) >> 4;

    /* Temperature in 0.1 °C */
    *temperature = T / 10.0f;

    return ESP_OK;
}

esp_err_t bmp180_read_pressure(float *pressure)
{
    if (bmp180_dev_handle == NULL) {
        ESP_LOGE(TAG, "Driver not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (pressure == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int32_t B5;

    esp_err_t err = bmp180_compute_b5(&B5);

    if (err != ESP_OK) {
        return err;
    }

    /* Start pressure conversion (OSS = 0) */
    uint8_t cmd[2] = {
        BMP180_REG_CTRL_MEAS,
        BMP180_CMD_PRESSURE_OSS0
    };

    err = i2c_master_transmit(
        bmp180_dev_handle,
        cmd,
        sizeof(cmd),
        -1
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start pressure conversion");
        return err;
    }

    /* Wait for pressure conversion */
    vTaskDelay(pdMS_TO_TICKS(5));

    uint8_t reg = BMP180_REG_OUT_MSB;
    uint8_t data[3];

    err = i2c_master_transmit_receive(
        bmp180_dev_handle,
        &reg,
        1,
        data,
        sizeof(data),
        -1
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read pressure");
        return err;
    }

    int32_t UP =
        (((int32_t)data[0] << 16) |
         ((int32_t)data[1] << 8)  |
         (int32_t)data[2]) >> 8;

    int32_t B6 = B5 - 4000;

    int32_t X1 =
        (calib.B2 * ((B6 * B6) >> 12)) >> 11;

    int32_t X2 =
        (calib.AC2 * B6) >> 11;

    int32_t X3 = X1 + X2;

    int32_t B3 =
        ((((int32_t)calib.AC1 * 4 + X3) + 2)) >> 2;

    X1 =
        (calib.AC3 * B6) >> 13;

    X2 =
        (calib.B1 * ((B6 * B6) >> 12)) >> 16;

    X3 =
        ((X1 + X2) + 2) >> 2;

    uint32_t B4 =
        (calib.AC4 * (uint32_t)(X3 + 32768)) >> 15;

    uint32_t B7 =
        ((uint32_t)UP - B3) * 50000;

    int32_t p;

    if (B7 < 0x80000000) {
        p = (B7 << 1) / B4;
    } else {
        p = (B7 / B4) << 1;
    }

    X1 =
        (p >> 8) * (p >> 8);

    X1 =
        (X1 * 3038) >> 16;

    X2 =
        (-7357 * p) >> 16;

    p =
        p + ((X1 + X2 + 3791) >> 4);

    /* Convert Pa to hPa */
    *pressure = p / 100.0f;

    return ESP_OK;
}

