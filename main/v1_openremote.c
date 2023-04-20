/**
 * Aplication Entry point
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "secret.h"
#include "wifi_app.h"

#include "bmp280.h"
#include "Cap_soil_moist.h"
#include "rgb_led.h"

#define WIFI_SSID         "Raptor GN"
#define WIFI_PASSWORD     "r4pt0rTECH"
#define MQTT_SERVER       "mqtt://192.168.2.188:1883"
#define MQTT_PORT         1883
#define MQTT_USERNAME     "master:mqttuser"
#define MQTT_PASSWORD     "78Z9dhK03upa3FlQLTmraS98cb1rlXZq"
#define CLIENT_ID         "First_test_Client"
#define TOPIC             "master/First_test_Client/writeattributevalue/Subscribe_attribute/7Y8SXoNL35Tu1t0Z33Ewfu"

static const char *TAG = "mqtt_example";
esp_mqtt_client_handle_t client;

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.2.188:1883",
        .credentials.client_id = CLIENT_ID,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
        // .event_handle = mqtt_event_handler,
        // .cert_pem = (const char *)server_cert_pem_start, // set your server certificate here
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void Sensors_read_task(void)
{
    int msg_id;
    bool value = true;
    char txt[11];
    int val = 1;

    while(1)
    {
        sprintf(txt, "%d", value);
        msg_id = esp_mqtt_client_publish(client, TOPIC, txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d",(int)sample_data().temperature);
        ESP_LOGD(TAG,"temperature:%lf", sample_data().temperature );
        // sprintf(txt, "%d", val);
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client/writeattributevalue/Write_attribute/7Y8SXoNL35Tu1t0Z33Ewfu", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d",(int)sample_data().pressure);
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client/writeattributevalue/pressure/7Y8SXoNL35Tu1t0Z33Ewfu", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d",moist_sens_val());
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client/writeattributevalue/humidity/7Y8SXoNL35Tu1t0Z33Ewfu", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        value = !value;
        val++;

        ESP_LOGI(TAG, "value: %d", value);
        ESP_LOGI(TAG, "val: %d", val);

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }

}

void app_main()
{
  // Initialize nvs
  esp_err_t ret = nvs_flash_init();
  if( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND )  // error check if found -> erase -> reinitialize
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();   
  }
    ESP_ERROR_CHECK(ret);

    rgb_led_init();
    moist_sens_init();
    my_i2c_setup();
    my_bme280_init(); // if it is now working, is the wrong device address hardcoded in 3 places, read,write,global

    wifi_app_start(); // starts and connects to wifi with credentials declared in wifi_app.h
    rgb_led_setState(eRgbLed_state_allOff);
    // get_rest_function();
    mqtt_app_start();

    xTaskCreate(Sensors_read_task, "SensorsReadTask", 4096, NULL, 10, NULL);
    
    

    // while (1) {
        // sprintf(txt, "%d", value);
        // msg_id = esp_mqtt_client_publish(client, TOPIC, txt, 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        // sprintf(txt, "%d",(int)sample_data().temperature);
        // ESP_LOGD(TAG,"temperature:%lf", sample_data().temperature );
        // // sprintf(txt, "%d", val);
        // msg_id = esp_mqtt_client_publish(client, "master/First_test_Client/writeattributevalue/Write_attribute/7Y8SXoNL35Tu1t0Z33Ewfu", txt, 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        // value = !value;
        // val++;

        // ESP_LOGI(TAG, "value: %d", value);
        // ESP_LOGI(TAG, "val: %d", val);

        // vTaskDelay(10000 / portTICK_PERIOD_MS);
    // }



}