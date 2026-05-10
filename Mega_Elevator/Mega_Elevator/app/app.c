/*
 * app.c
 *
 * Created: 4/11/2026 1:32:33 AM
 *  Author: Hamidur
 */ 

#include "app.h"

#include "uart.h"
#include "delay.h"

#include "../elevator/elevator.h"
#include "../display/display.h"
#include "../input/keypad_input.h"
#include "../comm/i2c_master.h"
#include "../config/config.h"
#include "../sonar/sonar.h"
#include "../test/test.h"

typedef enum
{
	APP_MODE_NORMAL = 0,
	APP_MODE_TEST = 1
} AppMode;

static AppMode s_app_mode = APP_MODE_NORMAL;

static uint8_t app_is_test_mode_requested(void);

void app_init(void)
{
	setup_uart_io();
	
	display_init();
	keypad_input_init();
	i2c_master_init();
	DELAY_ms(200);

	if (app_is_test_mode_requested() != 0u)
	{
		s_app_mode = APP_MODE_TEST;
		display_clear();
		display_write("TEST MODE");
	}
	else
	{
		s_app_mode = APP_MODE_NORMAL;
		sonar_init();
		elevator_init();
	}
}

void app_task(void)
{
	if (s_app_mode == APP_MODE_TEST)
	{
		test_main();
		return;
	}

	elevator_task();
}

static uint8_t app_is_test_mode_requested(void)
{
	uint16_t elapsed_ms = 0u;
	uint8_t key = '\0';

	while (elapsed_ms < APP_TEST_MODE_HOLD_MS)
	{
		if ((keypad_input_get_key_if_pressed(&key) == 0u) || (key != (uint8_t)APP_TEST_MODE_BOOT_KEY))
		{
			return 0u;
		}

		DELAY_ms(APP_LOOP_TICK_MS);
		elapsed_ms = (uint16_t)(elapsed_ms + APP_LOOP_TICK_MS);
	}

	return 1u;
}