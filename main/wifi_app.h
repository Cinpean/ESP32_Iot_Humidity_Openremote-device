/*
 wifi_app.h
    
    Created on: 13.12.2022

*/

#ifndef MAIN_WIFI_APP_H_
#define MAIN_WIFI_APP_H_

#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define STA_SSID "AndroidAP45 8F" //"Raptor GN" TP-LINK_419A14
#define STA_PASSWORD "00448855" //"r4pt0rTECH" 23927438

// netif object for the station
extern esp_netif_t* esp_netif_sta;

// typedef enum wifi_app_message
// {
//     WIFI_APP_MSG_STAR
// }


// typedef struct wifi_app_queue_message
// {
//     wifi_app_message_e msgID;
// }wifi_app_queue_message_t;

/**
 * Start wifi task
*/
void wifi_app_start(void);

/**
 * Begins the scanning process
*/
void scann(void);

/**
 * Gets world date/time from http server
*/

void get_rest_function(void);

void put_rest_function(void);


#endif /* MAIN_WIFI_APP_H_ */