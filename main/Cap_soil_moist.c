/**
 * Capacitive soil moisture sensor on pin IO34
*/
#include "Cap_soil_moist.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "math.h"
// #include "tuya_log.h"

#include "stdio.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
static esp_adc_cal_characteristics_t adc1_chars;
int16_t new_avg = 1;

void moist_sens_init(void){
    
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 1100, &adc1_chars);
    
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    // esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

int moist_sens_val(void){
    
    int adc_val = 0;
    adc_val = abs(((adc1_get_raw(ADC1_CHANNEL_6)-1000)*99/1560+1)-100);
    return adc_val;

}

int battery_procentage(void){
    
    int adc_val = 0;
    adc_val = adc1_get_raw(ADC1_CHANNEL_0);
    int16_t voltage = esp_adc_cal_raw_to_voltage(adc_val, &adc1_chars)*3; //*3 becuase using a 3:1 voltage divider
    new_avg = (0.3 * voltage) + ((1 - 0.3) * new_avg); // Exponential moving average
    uint8_t battery_percentage = (uint8_t)(((new_avg - 3500)*100)/750); // calculating voltage to %
    // TY_LOGD  (" battery voltage %d", voltage);
    // TY_LOGD (" new battery average %d",new_avg);
    return battery_percentage;
    
}