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
#include <string.h>
#include "driver/ledc.h"

#define WIFI_SSID         "Raptor GN"
#define WIFI_PASSWORD     "r4pt0rTECH"
#define MQTT_SERVER       "mqtt://192.168.2.188:1883"
#define MQTT_PORT         1883
#define MQTT_USERNAME     "master:mqttuser"
#define MQTT_PASSWORD     "78Z9dhK03upa3FlQLTmraS98cb1rlXZq"
#define CLIENT_ID         "First_test_Client"
#define TOPIC             "master/First_test_Client/writeattributevalue/Subscribe_attribute/7Y8SXoNL35Tu1t0Z33Ewfu"
#define attribute1        "light_led"
#define attribute2        "pump1"
#define attribute3        "led_color" 

static const char *TAG = "mqtt_example";
esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    int hex_number0, hex_number1, hex_number2, hex_number3;
    char * pEnd;
    char szNumbers[9];
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client/attributevalue/light_led/7Y8SXoNL35Tu1t0Z33Ewfu", 0);
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client/attributevalue/pump1/7Y8SXoNL35Tu1t0Z33Ewfu", 0);
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client/attributevalue/led_color/7Y8SXoNL35Tu1t0Z33Ewfu", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if(strncmp(event->topic + 40 ,attribute1,strlen(attribute1))==0 && event->data[0] == 't')  rgb_led_setState(eRgbLed_state_allOn);
            else if(strncmp(event->topic + 40 ,attribute1,strlen(attribute1))==0 && event->data[0] == 'f') rgb_led_setState(eRgbLed_state_allOff);

        if(strncmp(event->topic + 40 ,attribute2,strlen(attribute2))==0 && event->data[0] == 't') rgb_led_setState(eRgbLed_state_pick_color);
            else if(strncmp(event->topic + 40 ,attribute2,strlen(attribute2))==0 && event->data[0] == 'f') rgb_led_setState(eRgbLed_state_allOff);

/**
 * Hex-to-dec converison for controlling color of the led with 8 bit color depth 
 * received data is a concatenated char that is composed by 3 8-bit hex values 
*/

        if(strncmp(event->topic + 40 ,attribute3,strlen(attribute3))==0 ) { strncpy(szNumbers, event->data, sizeof(szNumbers) - 1);
                                                                            // printf("data of sznumbers = %.*s\r\n",sizeof(szNumbers),szNumbers);
                                                                            szNumbers[sizeof(szNumbers) - 1] = '\0';
                                                                            szNumbers[sizeof(szNumbers)] = '\0';
                                                                            szNumbers[0]=szNumbers[2];
                                                                            szNumbers[1]=szNumbers[3];
                                                                            szNumbers[2]=' ';
                                                                            hex_number1=(int)strtol(&szNumbers,&pEnd,16); 
                                                                            szNumbers[2]=szNumbers[4];
                                                                            szNumbers[3]=szNumbers[5];
                                                                            szNumbers[4]=' ';
                                                                            hex_number2=(int)strtol(pEnd,&pEnd,16);
                                                                            szNumbers[4]=szNumbers[6];
                                                                            szNumbers[5]=szNumbers[7];
                                                                            szNumbers[6]=' ';
                                                                            hex_number3=(int)strtol(pEnd,&pEnd,16);
                                                                            // printf("nr1 %d, nr2 %d, nr3 %d.",hex_number1,hex_number2,hex_number3);
                                                                            rgbled_color(hex_number1,hex_number2,hex_number3);
                                                                          }

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


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
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
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
    rgb_led_setState(eRgbLed_state_mqttActivation);
    wifi_app_start(); // starts and connects to wifi with credentials declared in wifi_app.h
    
    // get_rest_function();
    mqtt_app_start();
    rgb_led_setState(eRgbLed_state_allOff);
    xTaskCreate(Sensors_read_task, "SensorsReadTask", 4096, NULL, 10, NULL);
    
}