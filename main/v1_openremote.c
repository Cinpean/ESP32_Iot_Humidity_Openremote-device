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
#include "time.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "sys/time.h"
#include "esp_sntp.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "secret.h"
#include "wifi_app.h"

#include "bmp280.h"
#include "Cap_soil_moist.h"
#include "rgb_led.h"
#include <string.h>
#include "driver/ledc.h"
#include "stdlib.h" 
#include "store_flash.h"

#include "string.h"

// #define WIFI_SSID         "AndroidAP45 8F" //  Raptor GN
// #define WIFI_PASSWORD     "00448855"   // r4pt0rTECH
// #define MQTT_SERVER       "mqtt://192.168.55.131:1883"  // mqtt://192.168.2.188:1883
// #define MQTT_PORT         1883
#define MQTT_USERNAME     "master:mqttuser2"
#define MQTT_PASSWORD     "djfeNgOclsueI01kwvtcGTY5WWV7gh0H" //"78Z9dhK03upa3FlQLTmraS98cb1rlXZq"  //BItD8U8wRQiWX8Bc6d1PPuaTvnQx9csz  YUtirONcHQFARJ9ImvRTgG4RZwt0PtGU
#define CLIENT_ID         "First_test_Client23"
#define TOPIC             "master/First_test_Client23/writeattributevalue/Subscribe_attribute/5vV2VDBMDTX0ETXl28Tq0z" //7Y8SXoNL35Tu1t0Z33Ewfu
#define attribute1        "light_led"
#define attribute2        "pump1"
#define attribute3        "led_color" 
#define attribute4        "irig_start"

static const char *TAG = "mqtt_example";
esp_mqtt_client_handle_t client;
long long timestamp_ms = 0;
time_t timestamp_sec;
struct tm timeinfo;
struct tm current_timeinfo;
char* endptr;
int8_t flag_start_ap ;


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
    int hex_number1, hex_number2, hex_number3;
    char * pEnd;
    char szNumbers[9];
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client23/attributevalue/light_led/5vV2VDBMDTX0ETXl28Tq0z", 0);
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client23/attributevalue/pump1/5vV2VDBMDTX0ETXl28Tq0z", 0);
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client23/attributevalue/led_color/5vV2VDBMDTX0ETXl28Tq0z", 0);
        msg_id = esp_mqtt_client_subscribe(client, "master/First_test_Client23/attributevalue/irig_start/5vV2VDBMDTX0ETXl28Tq0z", 0);
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
        if(strncmp(event->topic + 42 ,attribute1,strlen(attribute1))==0 && event->data[0] == 't')  rgb_led_setState(eRgbLed_state_allOn);
            else if(strncmp(event->topic + 42 ,attribute1,strlen(attribute1))==0 && event->data[0] == 'f') rgb_led_setState(eRgbLed_state_allOff);

        if(strncmp(event->topic + 42 ,attribute2,strlen(attribute2))==0 && event->data[0] == 't') rgb_led_setState(eRgbLed_state_pick_color);
            else if(strncmp(event->topic + 42 ,attribute2,strlen(attribute2))==0 && event->data[0] == 'f') rgb_led_setState(eRgbLed_state_allOff);
        
        if(strncmp(event->topic + 42 ,attribute4,strlen(attribute4))==0 ){
            timestamp_ms = strtoll(event->data, &endptr, 10);
            timestamp_sec = (time_t)(timestamp_ms / 1000);
            ESP_LOGI("DATE","din Handler %lld",timestamp_ms);
            save_time_to_nvs("current_tim",timestamp_sec); // saves to flash time set for irigaton / or else

        }

/**
 * Hex-to-dec converison for controlling color of the led with 8 bit color depth 
 * received data is a concatenated char that is composed by 3 8-bit hex values 
*/

        if(strncmp(event->topic + 42 ,attribute3,strlen(attribute3))==0 ) { strncpy(szNumbers, event->data, sizeof(szNumbers) - 1);
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
        .broker.address.uri = "mqtt://192.168.55.131:1883",  // mqtt://192.168.2.188:1883
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
    bool done_check = 0;

    
    timeinfo.tm_hour = 14;
    timeinfo.tm_min = 31;
    timeinfo.tm_sec = 20;

    while(1)
    {
        sprintf(txt, "%d", value);
        msg_id = esp_mqtt_client_publish(client, TOPIC, txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d",(int)sample_data().temperature);
    
        ESP_LOGD(TAG,"temperature:%lf", sample_data().temperature );
        // sprintf(txt, "%d", val);
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client23/writeattributevalue/temp/5vV2VDBMDTX0ETXl28Tq0z", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d",(int)sample_data().pressure);
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client23/writeattributevalue/pressure/5vV2VDBMDTX0ETXl28Tq0z", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        sprintf(txt, "%d",moist_sens_val());
        msg_id = esp_mqtt_client_publish(client, "master/First_test_Client23/writeattributevalue/humidity/5vV2VDBMDTX0ETXl28Tq0z", txt, 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        value = !value;
        val++;

        ESP_LOGI(TAG, "value: %d", value);
        ESP_LOGI(TAG, "val: %d", val);


        time_t now;
        time(&now);
        localtime_r(&now, &current_timeinfo);
        localtime_r(&timestamp_sec, &timeinfo);

        

        if (current_timeinfo.tm_hour == timeinfo.tm_hour && current_timeinfo.tm_min == timeinfo.tm_min && done_check == 0) {
            ESP_LOGI("TRIGGER","intrerupt START OCCURED");
            done_check = 1;
            ESP_LOGI("TRIGGER","%d",timeinfo.tm_min);
            // msg_id = esp_mqtt_client_publish(client, "master/First_test_Client23/writeattributevalue/pump2/5vV2VDBMDTX0ETXl28Tq0z", 1, 0, 1, 0);
            //  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        }

        if (current_timeinfo.tm_hour == timeinfo.tm_hour && current_timeinfo.tm_min == timeinfo.tm_min + 1 && done_check == 1) {
            ESP_LOGI("TRIGGER","intrerupt END OCCURED");
            done_check = 0;
           // ESP_LOGI ("DATE","read from memory %d", read_time_from_nvs("current_tim",&timestamp_sec)); // reads from flash
           // printf("%d",timeinfo.tm_min);
           // ESP_LOGI("TRIGGER","%d",timeinfo.tm_hour);
            // msg_id = esp_mqtt_client_publish(client, "master/First_test_Client23/writeattributevalue/pump2/5vV2VDBMDTX0ETXl28Tq0z", 1, 0, 1, 0);
            //  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        }
        printf("%d",timeinfo.tm_min);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }

}
void obtain_time(void)
{
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;
    

    int retry = 0;
    const int retry_count = 10;

    sntp_servermode_dhcp(1);
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(sntp_set_time_sync_notification_cb);
    
    #ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    #endif
    sntp_init();
    
    setenv("TZ", "EET-2EEST-3,M3.5.0/03:00:00,M10.5.0/04:00:00", 1);
	tzset();
    
    /* wait for time to be set */
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    time(&now);
    localtime_r(&now,&timeinfo);

    printf("%d",timeinfo.tm_year);
    if(timeinfo.tm_year > 75) {rgbled_color(0,0,0);}
    else {
        if(flag_start_ap == 0) 
            {
                flag_start_ap = 1; 
                printf("flag start %d",flag_start_ap );
                save_flag_to_nvs("flag",flag_start_ap);
                esp_restart();
            }
        }

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG,"current date/time is: %s", strftime_buf);

}


void app_main()
{

  // Initialize nvs
  esp_err_t ret = nvs_flash_init();
  if( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND )  // error check if found -> erase -> reinitialize
  {
    ESP_LOGI("NVS ERROR","nvs errors found -> erased memory");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();   
  }
    ESP_ERROR_CHECK(ret);

    ret = open_nvs();
    if (ret != ESP_OK) {
        printf("NVS open failed\n");
        return;
    }

    read_flag_from_nvs("flag",&flag_start_ap);
    
    int8_t lungime_ssid;
    int8_t lungime_pass;
    char string_ssid[32];
	char string_pass[32];
	
    read_flag_from_nvs("sid_len",&lungime_ssid);
    read_flag_from_nvs("pass_len",&lungime_pass);
    lungime_ssid++;
    ESP_LOGI(TAG,"Lungime ssid %d", lungime_ssid);
    ESP_LOGI(TAG,"Lungime pass %d", lungime_pass);
    
    read_creds_from_nvs("service_id",(char*)&string_ssid,(size_t *)&lungime_ssid  );
	read_creds_from_nvs("pass",(char*)&string_pass,(size_t *)&lungime_pass );

    ESP_LOGI("CONNECTING TO:","%s With password: %s",string_ssid,string_pass);	

    rgb_led_init();
    moist_sens_init();
    my_i2c_setup();
    my_bme280_init(); // if it is now working, is the wrong device address hardcoded in 3 places, read,write,global
    rgbled_color(255,0,0);
    printf("flag start %d \n",flag_start_ap );
    
    if(flag_start_ap == 0) 
    {
        wifi_app_start(string_ssid, string_pass);   // starts and connects to wifi with credentials declared in wifi_app.h
        obtain_time();      // connects to internet and sets the current date and time 
        mqtt_app_start();
        close_nvs();

        xTaskCreate(Sensors_read_task, "SensorsReadTask", 4096, NULL, 10, NULL);
    }
    else if(flag_start_ap == 1) {wifi_app_start_ap();}
        else if(flag_start_ap > 1) {save_flag_to_nvs("flag",1);}
    

    // get_rest_function();
    
    // rgb_led_setState(eRgbLed_state_allOff);
    
}