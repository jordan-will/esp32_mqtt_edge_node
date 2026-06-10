#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <stdint.h>

#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_FREQ_HZ 100000 //standard to BMP180

/** 
@brief: initialize the master bus I2C
@return esp_err_t ESP_OK if the bus is initialized successfully
*/

esp_err_t i2c_bus_init(void);

/**
 * @brief: add a new slave on i2c bus
 * @param[in] dev_addr i2c address of 7 bits
 * @param[out] out_handle pointer where the handle is created
 * @return esp_err_t ESP_OK if the slave is added successfully
 */

esp_err_t i2c_bus_add_device(uint16_t dev_addr, i2c_master_dev_handle_t *out_handle); 

#endif