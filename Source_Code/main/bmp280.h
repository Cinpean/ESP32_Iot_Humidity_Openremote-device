/**
 *  bmp280.h   
 *  
 *  Created on Mar 3, 2023
 *      Author:  
 * 
*/

#ifndef MAIN_BMP280_H_
#define MAIN_BMP280_H_

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "bme280.h"


void my_i2c_setup(void);

void my_bme280_init(void);

int get_data_temp_pres(void);

struct bme280_data sample_data();


#endif