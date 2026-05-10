/*
 * test.c
 *
 * Created: 4/12/2026 4:21:27 AM
 *  Author: Hamidur
 */ 

#include <stdint.h>
#include <stdio.h>

#include "../comm/i2c_slave.h"
#include "../led/led.h"
#include "../buzzer/buzzer.h"
#include "../fan/fan.h"
#include "../dotmatrix/dotmatrix.h"

#include "pins.h"
#include "delay.h"

static uint8_t fan_test_done = 0U;
static uint8_t led_test_done = 0U;
static uint8_t buzzer_test_done = 0U;
static uint8_t dotmatrix_test_done = 0U;

void fan_test(void)
{
	if (fan_test_done == 0U)
	{
		fan_on_for_ms(2000U);
		fan_test_done = 1U;
	}
}

void fan_test_reset(void)
{
	fan_test_done = 0U;
}

void led_test(void)
{
	if (led_test_done == 0U)
	{
		led_on(LED_RED);
		DELAY_ms(300);
		led_off(LED_RED);

		led_on(LED_BLUE);
		DELAY_ms(300);
		led_off(LED_BLUE);

		led_on(LED_YELLOW);
		DELAY_ms(300);
		led_off(LED_YELLOW);

		led_on(LED_GREEN);
		DELAY_ms(300);
		led_off(LED_GREEN);

		led_on_for_ms(700U);
		led_test_done = 1U;
	}
}

void led_test_reset(void)
{
	led_test_done = 0U;
}

void buzzer_test(void)
{
	if (buzzer_test_done == 0U)
	{
		buzzer_tone(440U);
		DELAY_ms(120);
		buzzer_tone(523U);
		DELAY_ms(120);
		buzzer_start_melody();
		buzzer_test_done = 1U;
	}
}

void buzzer_test_reset(void)
{
	buzzer_test_done = 0U;
}

void dotmatrix_test(void)
{
	if (dotmatrix_test_done == 0U)
	{
		dotmatrix_show_up_arrow();
		DELAY_ms(1000);

		dotmatrix_show_down_arrow();
		DELAY_ms(1000);

		dotmatrix_show_red_circle();
		DELAY_ms(1000);

		dotmatrix_show_checker_pattern();
		DELAY_ms(1000);

		dotmatrix_show_full_square_pattern();
		DELAY_ms(1000);

		dotmatrix_clear();
		dotmatrix_test_done = 1U;
	}
}

void dotmatrix_test_reset(void)
{
	dotmatrix_test_done = 0U;
	dotmatrix_clear();
}