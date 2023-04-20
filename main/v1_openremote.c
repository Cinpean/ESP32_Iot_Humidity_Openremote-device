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

  wifi_app_start(); // starts and connects to wifi with credentials declared in wifi_app.h
  get_rest_function();
  mqtt_app_start();
 int msg_id;
    bool value = true;
    char txt[11];
    int val = 1;

    while (1) {
        sprintf(txt, "%d", value);
        msg_id = esp_mqtt_client_publish(client, TOPIC, txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d", val);
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client/writeattributevalue/Write_attribute/7Y8SXoNL35Tu1t0Z33Ewfu", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        value = !value;
        val++;

        ESP_LOGI(TAG, "value: %d", value);
        ESP_LOGI(TAG, "val: %d", val);

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }



}