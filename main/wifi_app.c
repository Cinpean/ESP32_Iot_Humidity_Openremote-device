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
#include "http_server.h"

#include "string.h"

// Tag used for ESP serial console messages
static const char *TAG = "wifi_app";
esp_netif_t* esp_netif_ap  = NULL;


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


static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT)
	{
		switch (event_id)
		{
			case WIFI_EVENT_AP_START:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
				break;

			case WIFI_EVENT_AP_STOP:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
				break;

			case WIFI_EVENT_AP_STACONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
				break;

			case WIFI_EVENT_AP_STADISCONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
				break;

			case WIFI_EVENT_STA_START:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
				break;

			case WIFI_EVENT_STA_CONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
				break;

			case WIFI_EVENT_STA_DISCONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
				break;
		}
	}
	else if (event_base == IP_EVENT)
	{
		switch (event_id)
		{
			case IP_EVENT_STA_GOT_IP:
				ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
				break;
		}
	}
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
    printf("says it connected to AP");
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
 * Initializes the WiFi application event handler for WiFi and IP events.
 */
static void wifi_app_event_handler_init(void)
{
	// Event loop for the WiFi driver
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Event handler for the connection
	esp_event_handler_instance_t instance_wifi_event;
	esp_event_handler_instance_t instance_ip_event;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));

}
/**
 * initializes wifi to station mode
*/
void wifi_initial_sta()
{
  
  esp_netif_init(); // TCP/IP init
  esp_event_loop_create_default(); //event loop
  esp_netif_create_default_wifi_sta(); //wifi station
  // esp_netif_ap = esp_netif_create_default_wifi_ap(); // wifi acces point
  // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_init_config_t wifi_initialization = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_initialization);
}

static void wifi_app_default_wifi_ap_init(void)
{
	// Initialize the TCP stack
	ESP_ERROR_CHECK(esp_netif_init());

	// Default WiFi config - operations must be in this order!
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	// esp_netif_sta = esp_netif_create_default_wifi_sta();
	esp_netif_ap = esp_netif_create_default_wifi_ap();
}

/**
 * configures ssid and password for the accespoint where it connects
 * starts wifi connection
*/

void wifi_configur_sta(char *STATION_SSID, char *STATION_PASS)
{
 // esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
 // esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

  wifi_config_t wifi_configuration = {
    .sta = {
      .ssid = {0},
      .password =  {0},
      },
  };
    strncpy((char *)wifi_configuration.sta.ssid, STATION_SSID, sizeof(wifi_configuration.sta.ssid) - 1);
    strncpy((char *)wifi_configuration.sta.password, STATION_PASS, sizeof(wifi_configuration.sta.password) - 1);

//   ESP_LOGI("wifi_configur_sta","%s",wifi_configuration.sta.ssid);

  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
  esp_wifi_set_mode(WIFI_MODE_STA);
  
  ESP_ERROR_CHECK(esp_wifi_start());

  connection_status_return(esp_wifi_connect());
  // wifi_ap_record_t ap_info;
  // printf("%s \n",ap_info.ssid);
  // ESP_LOGI("WIFI_CONNECTED","%d",esp_wifi_sta_get_ap_info(&ap_info));
}

BaseType_t wifi_app_send_message(wifi_app_message_e msgID)
{
	wifi_app_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}


static void wifi_app_soft_ap_config(void)
{
	// SoftAP - WiFi access point configuration
	wifi_config_t ap_config =
	{
		.ap = {
				.ssid = WIFI_AP_SSID,
				.ssid_len = strlen(WIFI_AP_SSID),
				.password = WIFI_AP_PASSWORD,
				.channel = WIFI_AP_CHANNEL,
				.ssid_hidden = WIFI_AP_SSID_HIDDEN,
				.authmode = WIFI_AUTH_WPA2_PSK,
				.max_connection = WIFI_AP_MAX_CONNECTIONS,
				.beacon_interval = WIFI_AP_BEACON_INTERVAL,
		},
	};

	// Configure DHCP for the AP
	esp_netif_ip_info_t ap_ip_info;
	memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

	esp_netif_dhcps_stop(esp_netif_ap);					///> must call this first
	inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);		///> Assign access point's static IP, GW, and netmask
	inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
	inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
	ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));			///> Statically configure the network interface
	ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));						///> Start the AP DHCP server (for connecting stations e.g. your mobile device)

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));						///> Setting the mode as Access Point / Station Mode
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));			///> Set our configuration
	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_AP_BANDWIDTH));		///> Our default bandwidth 20 MHz
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));						///> Power save set to "NONE"

}

static void wifi_app_task(void * pvParameters)
{
	wifi_app_queue_message_t msg;

	// Initialize the event handler
	wifi_app_event_handler_init();

	// Initialize the TCP/IP stack and WiFi config
	wifi_app_default_wifi_ap_init();

	

	// SoftAP config
	wifi_app_soft_ap_config();

	// Start WiFi
	ESP_ERROR_CHECK(esp_wifi_start());

	// Send first event message
	wifi_app_send_message(WIFI_APP_MSG_START_HTTP_SERVER);

	for (;;)
	{
		if (xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case WIFI_APP_MSG_START_HTTP_SERVER:
					ESP_LOGI(TAG, "WIFI_APP_MSG_START_HTTP_SERVER");

					http_server_start();

					// rgb_led_http_server_started();

					break;

				case WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
					ESP_LOGI(TAG, "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER");
					http_server_monitor_send_message(HTTP_MSG_WIFI_CONNECT_INIT);

					break;

				case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
					ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");
					// rgb_led_wifi_connected();
					http_server_monitor_send_message(HTTP_MSG_WIFI_CONNECT_SUCCESS);

					break;

				default:
					break;

			}
		}
	}
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

void wifi_app_start (char *ssid, char *pass)
{
    ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

    wifi_initial_sta();
    wifi_configur_sta(ssid, pass);

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  


   // xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task" , WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID );
}

void wifi_app_start_ap(void)
{
	ESP_LOGI(TAG, "STARTING WIFI APPLICATION");

	// Start WiFi started LED
	// rgb_led_wifi_app_started();

	// Disable default WiFi logging messages
	esp_log_level_set("wifi", ESP_LOG_NONE);

	// Create message queue
	wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

	// Start the WiFi application task
	xTaskCreatePinnedToCore(&wifi_app_task, "wifi_app_task", WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_TASK_CORE_ID);
}