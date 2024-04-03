/**
 * Cap_soil_moist.h
 * 
 *  Created on Feb 6, 2023
 *  Author: 
 *      
*/

#ifndef MAIN_CAP_SOIL_MOIST_H
#define MAIN_CAP_SOIL_MOIST_H

#include "esp_adc_cal.h"

static esp_adc_cal_characteristics_t adc1_chars;

void moist_sens_init(void);
int moist_sens_val(void);
int battery_procentage(void);


#endif /* MAIN_CAP_SOIL_MOIST */