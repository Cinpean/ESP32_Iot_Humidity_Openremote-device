/*
 * rgb_led.h
 *
 *  Created on: Jan 11, 2023
 *      Author: alexandru.bogdan
 */

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

typedef enum
{
	eRgbLed_state_init,
	eRgbLed_state_wifiConnected,
	eRgbLed_state_mqttActivation,
	eRgbLed_state_allOn,
	eRgbLed_state_allOff,
	eRgbLed_state_pick_color,
	eRgbLed_state_pick_color2,
}eRgbLed_state_t;

void rgb_led_init(void);
void rgb_led_setState(eRgbLed_state_t state);
void rgbled_color(int red, int green, int blue );


#endif /* MAIN_RGB_LED_H_ */
