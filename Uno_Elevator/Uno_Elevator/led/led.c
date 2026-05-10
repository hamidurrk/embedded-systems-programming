/*
 * led.c
 *
 * Created: 4/11/2026 1:47:49 AM
 *  Author: Hamidur
 */ 

#include "led.h"

#include "pins.h"
#include "delay.h"
#include "gpio.h"

void led_init(void)
{
	pin_mode(LED_RED, OUTPUT);
	pin_mode(LED_BLUE, OUTPUT);
	pin_mode(LED_YELLOW, OUTPUT);
	pin_mode(LED_GREEN, OUTPUT);

	digital_write(LED_RED, LOW);
	digital_write(LED_BLUE, LOW);
	digital_write(LED_YELLOW, LOW);
	digital_write(LED_GREEN, LOW);
}

void led_on(uint8_t pin)
{
	pin_mode(pin, OUTPUT);
	digital_write(pin, HIGH);
}

void led_off(uint8_t pin)
{
	pin_mode(pin, OUTPUT);
	digital_write(pin, LOW);
}

void led_on_for_ms(uint16_t duration_ms)
{
	led_on(LED_RED);
	led_on(LED_BLUE);
	led_on(LED_YELLOW);
	led_on(LED_GREEN);

	while (duration_ms > 0U)
	{
		DELAY_ms(1);
		duration_ms--;
	}

	led_off(LED_RED);
	led_off(LED_BLUE);
	led_off(LED_YELLOW);
	led_off(LED_GREEN);
}

void led_all_on(void)
{
	led_on(LED_RED);
	led_on(LED_BLUE);
	led_on(LED_YELLOW);
	led_on(LED_GREEN);
}

void led_all_off(void)
{
	led_off(LED_RED);
	led_off(LED_BLUE);
	led_off(LED_YELLOW);
	led_off(LED_GREEN);
}