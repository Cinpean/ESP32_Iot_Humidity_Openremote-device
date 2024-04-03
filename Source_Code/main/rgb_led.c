#include "rgb_led.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

void rgb_led_init(void)
{
	// gpio_pad_select_gpio(GPIO_NUM_5);
	gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
	// // gpio_pad_select_gpio(GPIO_NUM_18);
    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT);
	// // gpio_pad_select_gpio(GPIO_NUM_17);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

	ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_8_BIT,
    .freq_hz = 5000,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_num = LEDC_TIMER_0,
	 .clk_cfg = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel_config_t ledc_channel_red = {
    .channel = LEDC_CHANNEL_0,
    .duty = 0,
    .gpio_num = GPIO_NUM_5,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_sel = LEDC_TIMER_0
	};
	ledc_channel_config(&ledc_channel_red);

	ledc_channel_config_t ledc_channel_green = {
    .channel = LEDC_CHANNEL_2,
    .duty = 0,
    .gpio_num = GPIO_NUM_18,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_sel = LEDC_TIMER_0
	};
	ledc_channel_config(&ledc_channel_green);

	ledc_channel_config_t ledc_channel_blue = {
    .channel = LEDC_CHANNEL_4,
    .duty = 0,
    .gpio_num = GPIO_NUM_17,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_sel = LEDC_TIMER_0
	};
	ledc_channel_config(&ledc_channel_blue);

}


void rgb_led_setState(eRgbLed_state_t state)
{
	switch(state)
	{
	case eRgbLed_state_init:
	    gpio_set_level(GPIO_NUM_5, 1);
	    gpio_set_level(GPIO_NUM_18, 0);
	    gpio_set_level(GPIO_NUM_17, 0);
		break;
	case eRgbLed_state_wifiConnected:
	    gpio_set_level(GPIO_NUM_5, 0);
	    gpio_set_level(GPIO_NUM_18, 1);
	    gpio_set_level(GPIO_NUM_17, 0);
		break;
	case eRgbLed_state_mqttActivation:
	    gpio_set_level(GPIO_NUM_5, 0);
	    gpio_set_level(GPIO_NUM_18, 0);
	    gpio_set_level(GPIO_NUM_17, 1);
		break;
	case eRgbLed_state_allOn:
	    gpio_set_level(GPIO_NUM_5, 1);
	    gpio_set_level(GPIO_NUM_18, 1);
	    gpio_set_level(GPIO_NUM_17, 1);
		break;
	case eRgbLed_state_allOff:
	    gpio_set_level(GPIO_NUM_5, 0);
	    gpio_set_level(GPIO_NUM_18, 0);
	    gpio_set_level(GPIO_NUM_17, 0);
		break;
		
	case eRgbLed_state_pick_color:
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 250);
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 150);
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4, 60);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4);
		break;

	case eRgbLed_state_pick_color2:
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 150);
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 250);
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4, 110);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4);
		break;

	default:
		break;
	}
}

void rgbled_color(int red, int green, int blue )
{
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, red);
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, green);
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4, blue);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_4);
}