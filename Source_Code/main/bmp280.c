// implementations of all the stuff the Bosch BME280 reference code needs implemented 
// in order to talk to an actual bme280 over i2c on esp32 in idf
// -- github.com/epvuc 12/2017

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "bme280.h"
#include "bmp280.h"
// #include "esp_log.h"

#define SCL_PIN 22
#define SDA_PIN 21

int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
int8_t stream_sensor_data_forced_mode(struct bme280_dev *dev);
int8_t stream_sensor_data_normal_mode(struct bme280_dev *dev);
void user_delay_ms(uint32_t ms);

struct bme280_dev bme280;

void my_i2c_setup(void)
{

    gpio_set_pull_mode(SDA_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SCL_PIN, GPIO_PULLUP_ONLY);
    gpio_set_direction(SDA_PIN, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_direction(SCL_PIN, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(SDA_PIN, 1);
    gpio_set_level(SCL_PIN, 1);


  i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE, // ENABLE
		.scl_pullup_en = GPIO_PULLUP_ENABLE, // ENABLE
		.master.clk_speed = 100000};
  printf("calling i2c setup");
  i2c_param_config(I2C_NUM_0, &conf);
  printf("between config & install");
  i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
//   printf("done i2c setup");
} 

void my_bme280_init(void)
{
  int8_t rslt = BME280_OK;
  uint8_t settings_sel;
  
  bme280.chip_id = 0x76;
  bme280.intf = BME280_I2C_INTF;
  bme280.read = (void *)user_i2c_read;
  bme280.write = (void *)user_i2c_write;
  bme280.delay_us = (void *)user_delay_ms;

 //

  bme280.settings.osr_h = BME280_OVERSAMPLING_1X;
  bme280.settings.osr_p = BME280_OVERSAMPLING_4X;
  bme280.settings.osr_t = BME280_OVERSAMPLING_4X;
  bme280.settings.filter = BME280_FILTER_COEFF_OFF;

  printf("\r\n calling bme280_init  \r\n");
  rslt = bme280_init(&bme280);
  printf("bme280 init result %d\r\n", rslt);


  settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
  rslt = bme280_set_sensor_settings(settings_sel, &bme280);
  printf("bme280 settings config result %d\r\n", rslt);
}

#define SUCCESS 0
#define FAIL -1
#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1

int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
  int32_t iError = 0;

  esp_err_t espRc;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);

  i2c_master_write_byte(cmd, reg_addr, true);
  i2c_master_write(cmd, reg_data, cnt, true);
  i2c_master_stop(cmd);

  espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
  //  printf("in user_i2c_write, i2c_master_cmd_begin returns %d\r\n", espRc);
  if (espRc == ESP_OK) {
    iError = SUCCESS;
  } else {
    iError = FAIL;
  }
  i2c_cmd_link_delete(cmd);

  return (int8_t)iError;
}

int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
  int32_t iError = 0;
  esp_err_t espRc;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg_addr, true);

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

  if (cnt > 1) {
    i2c_master_read(cmd, reg_data, cnt-1, I2C_MASTER_ACK);
  }
  i2c_master_read_byte(cmd, reg_data+cnt-1, I2C_MASTER_NACK);
  i2c_master_stop(cmd);

  espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
  if (espRc == ESP_OK) {
    iError = SUCCESS;
  } else {
    iError = FAIL;
  }


  i2c_cmd_link_delete(cmd);

  return (int8_t)iError;
}

void user_delay_ms(uint32_t ms)
{
  vTaskDelay(ms/portTICK_PERIOD_MS);
}
int get_data_temp_pres(void)
{
	// i2c_master_init();
	// xTaskCreate(&task_bme280_normal_mode, "bme280_normal_mode",  2048, NULL, 6, NULL);
	// // xTaskCreate(&task_bme280_forced_mode, "bme280_forced_mode",  2048, NULL, 6, NULL);
    int8_t rslt;
    uint32_t count = 0;

    struct bme280_data comp_data;
    
    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &bme280);
    bme280.delay_us(40,BME280_I2C_INTF);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &bme280);
    printf("temp: %f, preasure:%f \r\n",comp_data.temperature, comp_data.pressure);
    vTaskDelay(100);
  
  return (int)comp_data.temperature;
}


struct bme280_data sample_data(){
  
  int8_t rslt; 
  struct bme280_data comp_data;

    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &bme280);
    bme280.delay_us(40,BME280_I2C_INTF);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &bme280);
    /*
    // because of how the pcb is done, heat generated by esp32 interpheres with measurement and the average power consumntion is 90mAp at 3.3V =>
     (with ΔT = P * Rth - where ΔT is the temperature rise, P is the power dissipation, and Rth is the thermal resistance. )
     This means that the ESP32 temperature will increase by 5.6 °C above the ambient temperature.
    */
    comp_data.temperature -= 5;  
    comp_data.pressure /= 100;
    return comp_data;  

}