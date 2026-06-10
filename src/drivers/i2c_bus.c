#include "../include/i2c_bus.h"
#include "esp_log.h"

static const char *TAG = "I2C_BUS";
//handle bus controller
static i2c_master_bus_handle_t bus_handle = NULL;

esp_err_t i2c_bus_init(void){
    if(bus_handle != NULL) return ESP_OK;

    //initial config of the bus
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,//DEFAULT CLOCK SYSTEM
        .i2c_port = I2C_NUM_0,//PORT HARDWARE I2C0
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,//FILTER AGAINST GLITCH
        .flags.enable_internal_pullup = true //redundant security
    };

    //initialize the physical bus 
    esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "INITIALIZE I2C MASTER BUS FAILED: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C MASTER BUS INITIALIZED SUCCESSFULLY");
    return ESP_OK;
}

esp_err_t i2c_bus_add_device(uint16_t dev_addr, i2c_master_dev_handle_t *out_handle){
    if(bus_handle == NULL){
        ESP_LOGE(TAG, "Attempt add a device before initializing the bus");
        return ESP_ERR_INVALID_STATE;
    }

    //config a specific device that it will be conected on bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, //default address of 7 bits
        .device_address = dev_addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ
    };

    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_config, out_handle);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "FAIL ON REGISTER NEW DEVICE ON ADDRESS 0x%02X: %s", dev_addr, esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "NEW DEVICE RESGISTERED ON ADDRESS 0x%02X SUCCESSFULLY", dev_addr);
    return ESP_OK;
}



