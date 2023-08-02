#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "time.h"



esp_err_t init_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

nvs_handle_t nvs_handler;

esp_err_t open_nvs() {
    esp_err_t ret = nvs_open("storage", NVS_READWRITE, &nvs_handler);
    return ret;
}

void close_nvs() {
    nvs_close(nvs_handler);
}

esp_err_t save_time_to_nvs(const char* key, time_t value) {
    // open_nvs();
    esp_err_t ret = nvs_set_i64(nvs_handler, key, value);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handler);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    if (ret != ESP_OK) {
           printf("Failed to save time to NVS\n");
       }
    else printf("done save time to NVS\n");
    // close_nvs();
    return ret;
}

esp_err_t read_time_from_nvs(const char* key, time_t* value) {
    // open_nvs();
    esp_err_t ret = nvs_get_i64(nvs_handler, key, value);
    // close_nvs();
    return ret;
}





void save_time_task( time_t current_time) {
    
        esp_err_t ret = save_time_to_nvs("current_time", current_time);
        if (ret != ESP_OK) {
            printf("Failed to save time to NVS\n");
        }
        else printf("done save time to NVS\n");

        //vTaskDelay(5000 / portTICK_PERIOD_MS); // Save the time every 5 seconds
    
}