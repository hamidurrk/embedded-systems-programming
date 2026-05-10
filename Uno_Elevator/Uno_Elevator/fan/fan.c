/*
 * fan.c
 *
 * Created: 4/12/2026 3:32:40 AM
 *  Author: Hamidur
 */ 

#include "fan.h"

#include "pins.h"
#include "delay.h"
#include "gpio.h"

void fan_init(void)
{
	pin_mode(FAN, OUTPUT);
}

void fan_on(void)
{
	fan_init();
	digital_write(FAN, HIGH);
}

void fan_off(void)
{
	fan_init();
	digital_write(FAN, LOW);
}

void fan_on_for_ms(uint16_t duration_ms)
{
	fan_on();

	while (duration_ms > 0U)
	{
		DELAY_ms(1);
		duration_ms--;
	}

	fan_off();
}
