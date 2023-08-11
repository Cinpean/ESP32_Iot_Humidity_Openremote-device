/*
 * http_server.c
 *
 *  Created on: Oct 20, 2021
 *      Author: kjagu
 */

#include "esp_http_server.h"
#include "esp_log.h"

#include "http_server.h"
#include "tasks_common.h"
#include "wifi_app.h"
#include "store_flash.h"

int8_t flag_start_ap_http_srv ;


// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

static int g_wifi_connect_status = 0;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

// Embedded files: JQuery, index.html, app.css, app.js and favicon.ico files
extern const uint8_t jquery_3_3_1_min_js_start[]	asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[]		asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[]				asm("_binary_index_html_start");
extern const uint8_t index_html_end[]				asm("_binary_index_html_end");
extern const uint8_t app_css_start[]				asm("_binary_app_css_start");
extern const uint8_t app_css_end[]					asm("_binary_app_css_end");
extern const uint8_t app_js_start[]					asm("_binary_app_js_start");
extern const uint8_t app_js_end[]					asm("_binary_app_js_end");
// extern const uint8_t favicon_ico_start[]			asm("_binary_favicon_ico_start");
// extern const uint8_t favicon_ico_end[]				asm("_binary_favicon_ico_end");

/**
 * HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	for (;;)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case HTTP_MSG_WIFI_CONNECT_INIT:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECTING;
					break;

				case HTTP_MSG_WIFI_CONNECT_SUCCESS:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_SUCCESS;
					break;

				case HTTP_MSG_WIFI_CONNECT_FAIL:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_FAILED;
					break;

				case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_SUCCESSFUL");

					break;

				case HTTP_MSG_OTA_UPDATE_FAILED:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPDATE_FAILED");

					break;

				case HTTP_MSG_OTA_UPATE_INITIALIZED:
					ESP_LOGI(TAG, "HTTP_MSG_OTA_UPATE_INITIALIZED");

					break;

				default:
					break;
			}
		}
	}
}

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Jquery requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}

/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "index.html requested");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}

/**
 * app.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.css requested");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

	return ESP_OK;
}

/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

	return ESP_OK;
}

/**
 * Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
// static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
// {
// 	ESP_LOGI(TAG, "favicon.ico requested");

// 	httpd_resp_set_type(req, "image/x-icon");
// 	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

// 	return ESP_OK;
// }

/**
 * wificonnect.json handler invoked after button pressed and handles receiving the ssid and pwd enterd by user
 * return ESP_OK
*/
static esp_err_t http_server_wifi_connect_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG,"/wifiConnect.json requested");
	size_t len_ssid = 0, len_pass = 0;
	char *ssid_str = NULL , *pass_str = NULL; 


	//Get SSID header
	len_ssid = httpd_req_get_hdr_value_len(req, "sid" ) + 1;
	if (len_ssid > 1)
	{
		ssid_str = malloc(len_ssid);
		if (httpd_req_get_hdr_value_str(req, "sid", ssid_str, len_ssid) == ESP_OK)
		{
			ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => ssid: %s", ssid_str);
		}
	}
	else ESP_LOGI(TAG,"ssid %d",len_ssid);

	ESP_LOGI(TAG,"after ssid");

	len_pass = httpd_req_get_hdr_value_len(req, "pwd") + 1;
	if (len_pass > 1)
	{
		ESP_LOGI(TAG,"len pass > 1");
		pass_str = malloc(len_pass);
		ESP_LOGI(TAG,"malloc > 1");
		if (httpd_req_get_hdr_value_str(req, "pwd", pass_str, len_pass) == ESP_OK)
		{
			ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => pwd: %s", pass_str);
		}
	}
	else { ESP_LOGI(TAG,"after pwd %d",len_pass); flag_start_ap_http_srv = 1; }
	
	if(pass_str != NULL)
	{  
	// ESP_LOGI(TAG," flag_start_ap_http_srv %d",flag_start_ap_http_srv);
	flag_start_ap_http_srv = 0;
	save_flag_to_nvs("flag",flag_start_ap_http_srv);
	// read_flag_from_nvs("flag",&flag_start_ap_http_srv);
	// ESP_LOGI(TAG," flag_start_ap_http_srv %d",flag_start_ap_http_srv);

	/**
	 * saves to flash memory the station credentials 
	*/
	save_creds_to_nvs("service_id",ssid_str);		
	save_creds_to_nvs("pass",pass_str);
	save_flag_to_nvs("sid_len",len_ssid);
	save_flag_to_nvs("pass_len",len_pass);

	// char string_ssid[32];
	// char string_pass[32];
	// ESP_LOGI(TAG,"MUIUI %d",len_ssid -1 );
	// read_creds_from_nvs("service_id",(char*)&string_ssid,&len_ssid-1);
	// read_creds_from_nvs("pass",(char*)&string_pass,&len_pass-1);

	close_nvs();
	esp_restart();
	}

	
	// wifi_app_send_message(WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER);
	
	g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_SUCCESS;


	char statusJSON[100];

	sprintf(statusJSON, "{\"wifi_connect_status\":%d}", 1);

	ESP_LOGI(TAG," nu e in handler %s",statusJSON);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, statusJSON, strlen(statusJSON));


	return ESP_OK;

}


static esp_err_t http_server_wifi_connect_status_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnectStatus requested");
	
	g_wifi_connect_status =  HTTP_WIFI_STATUS_CONNECTING;;

	char statusJSON[100];

	sprintf(statusJSON, "{\"wifi_connect_status\":%d}", g_wifi_connect_status);

	ESP_LOGI(TAG,"%s",statusJSON);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, statusJSON, strlen(statusJSON));

	return ESP_OK;
}

/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
	// Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Create HTTP server monitor task
	xTaskCreatePinnedToCore(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor, HTTP_SERVER_MONITOR_CORE_ID);

	// Create the message queue
	http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

	// The core that the HTTP server will run on
	config.core_id = HTTP_SERVER_TASK_CORE_ID;

	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	ESP_LOGI(TAG,
			"http_server_configure: Starting server on port: '%d' with task priority: '%d'",
			config.server_port,
			config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");

		// register query handler
		httpd_uri_t jquery_js = {
				.uri = "/jquery-3.3.1.min.js",
				.method = HTTP_GET,
				.handler = http_server_jquery_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &jquery_js);

		// register index.html handler
		httpd_uri_t index_html = {
				.uri = "/",
				.method = HTTP_GET,
				.handler = http_server_index_html_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &index_html);

		// register app.css handler
		httpd_uri_t app_css = {
				.uri = "/app.css",
				.method = HTTP_GET,
				.handler = http_server_app_css_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_css);

		// register app.js handler
		httpd_uri_t app_js = {
				.uri = "/app.js",
				.method = HTTP_GET,
				.handler = http_server_app_js_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_js);

		

		// register favicon.ico handler
		// httpd_uri_t favicon_ico = {
		// 		.uri = "/favicon.ico",
		// 		.method = HTTP_GET,
		// 		.handler = http_server_favicon_ico_handler,
		// 		.user_ctx = NULL
		// };
		// httpd_register_uri_handler(http_server_handle, &favicon_ico);

		// register wifiConnect.json handler
		httpd_uri_t wifi_connect_json = {
				.uri = "/wificonnect.json",
				.method = HTTP_POST,
				.handler = http_server_wifi_connect_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_json);

		// register wifiConnectStatus.json handler
		httpd_uri_t wifi_connect_status_json = {
				.uri = "/wifiConnectStatus",
				.method = HTTP_POST,
				.handler = http_server_wifi_connect_status_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_status_json);


		return http_server_handle;
	}

	return NULL;
}

void http_server_start(void)
{
	if (http_server_handle == NULL)
	{
		http_server_handle = http_server_configure();
	}
}

void http_server_stop(void)
{
	if (http_server_handle)
	{
		httpd_stop(http_server_handle);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
		http_server_handle = NULL;
	}
	if (task_http_server_monitor)
	{
		vTaskDelete(task_http_server_monitor);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
		task_http_server_monitor = NULL;
	}
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
	http_server_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}

int return_flag_ap (){
	return flag_start_ap_http_srv ;
}

















