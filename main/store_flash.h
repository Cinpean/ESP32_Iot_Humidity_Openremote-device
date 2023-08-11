#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

void save_time_task(time_t current_time);

void close_nvs(void);

esp_err_t open_nvs(void);

esp_err_t init_nvs(void);

esp_err_t save_time_to_nvs(const char* key, time_t value);

esp_err_t read_time_from_nvs(const char* key, time_t* value);

esp_err_t save_flag_to_nvs(const char* key, int8_t value);

esp_err_t read_flag_from_nvs(const char* key, int8_t* value);

esp_err_t save_creds_to_nvs(const char* key, const char* value);

esp_err_t read_creds_from_nvs(const char* key, char* value,size_t buffer_size);
