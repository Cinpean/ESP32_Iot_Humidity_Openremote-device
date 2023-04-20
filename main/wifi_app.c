/*
 * wifi_app.c
 *
 *  Created on: 13.12.2022
*/


#include "wifi_app.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "esp_err.h"

#include "credentials.h"
#include "tasks_common.h"

// Tag used for ESP serial console messages
static const char *TAG = "wifi_app";

//queue handle used to manipulate the main queue of events
//static QueueHandle_t wifi_app_queue_handle;


void scann(){ 
  wifi_scan_config_t scan_config = {
    .ssid = 0,
    .bssid = 0,
    .channel = 0,
        .show_hidden = true
    };
  printf("Start scanning...");
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
  printf(" completed!\n");
 
  // get the list of APs found in the last scan
  uint16_t ap_num;
  wifi_ap_record_t ap_records[20];
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
  
  // print the list 
  printf("Found %d access points:\n", ap_num);

  printf("               SSID              | Channel | RSSI |   MAC \n\n");
  
  for(int i = 0; i < ap_num; i++)
    printf("%32s | %7d | %4d   %2x:%2x:%2x:%2x:%2x:%2x   \n", ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi , *ap_records[i].bssid, *(ap_records[i].bssid+1), *(ap_records[i].bssid+2), *(ap_records[i].bssid+3), *(ap_records[i].bssid+4), *(ap_records[i].bssid+5));
}

/**
 * Wifi app status return when connecting to AP
*/

static void connection_status_return (int connmode)
{
  switch (connmode)
  {
  case ESP_OK:
    printf("ESP_OK Connected \n");
    break;

    case ESP_ERR_WIFI_NOT_INIT:
    printf("WiFi is not initialized by esp_wifi_init \n");
    break;
  
   case ESP_ERR_WIFI_NOT_STARTED:
    printf("WiFi is not started by esp_wifi_start \n");
    break;
    
   case ESP_ERR_WIFI_CONN:
    printf("WiFi internal error, station or soft-AP control block wrong \n");
    break;

   case ESP_ERR_WIFI_SSID:
    printf("SSID of AP which station connects is invalid \n");
    break;

  default:
    printf("default");
    break;
  }
}

/**
 * wifi http get handler > prints on terminal data from server
*/

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
  switch (evt->event_id)
  {
  case HTTP_EVENT_ON_DATA:
    printf("EVENT %.*s \n", evt->data_len, (char *)evt->data );
    break;

  case HTTP_EVENT_ON_CONNECTED:
    printf("connected to server \n");
    break;
  
  default:
    break;
  }
  return ESP_OK;
}

/**
 * initializes wifi to station mode
*/

void wifi_initial()
{
  
  esp_netif_init(); // TCP/IP init
  esp_event_loop_create_default(); //event loop
  esp_netif_create_default_wifi_sta(); //wifi station
  wifi_init_config_t wifi_initialization = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_initialization);
}

/**
 * configures ssid and password for the accespoint where it connects
 * starts wifi connection
*/

void wifi_configur()
{
 // esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
 // esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
  wifi_config_t wifi_configuration = {
    .sta = {
      .ssid = STA_SSID,
      .password = STA_PASSWORD
      }};

  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
  esp_wifi_set_mode(WIFI_MODE_STA);
  
  ESP_ERROR_CHECK(esp_wifi_start());

  connection_status_return(esp_wifi_connect());
}

/**
 * gets data from http server 
*/

void get_rest_function()
{
  esp_http_client_config_t client_configuration = {
    .url = server_get_url,
    .method = HTTP_METHOD_GET,
    .cert_pem = NULL,
    .event_handler = client_event_get_handler
  };
  esp_http_client_handle_t client = esp_http_client_init(&client_configuration);
  esp_http_client_perform(client);
  esp_http_client_cleanup(client);
}

/**
 * sends Char data to http server
*/

void put_rest_function()
{
 esp_http_client_config_t client_configuration = {
    .url = "http://httpbin.org/post",
    .method = HTTP_METHOD_POST,
    .cert_pem = NULL,
    .event_handler = client_event_get_handler
  };

  esp_http_client_handle_t client = esp_http_client_init(&client_configuration);
  char *post_data = "tesste..a";

  esp_http_client_set_post_field(client, post_data, strlen(post_data));
  esp_http_client_set_header(client, "Content-Type", "application/json");
  
  esp_http_client_perform(client);
  esp_http_client_cleanup(client);
}

void wifi_app_start ()
{
    ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

    wifi_initial();
    wifi_configur();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  


   // xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task" , WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID );
}