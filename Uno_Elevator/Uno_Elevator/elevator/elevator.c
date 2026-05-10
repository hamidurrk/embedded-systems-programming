/*
 * elevator.c
 *
 * Created: 4/12/2026 9:14:45 PM
 *  Author: Hamidur
 */ 

#include "elevator.h"

#include <avr/sleep.h>
#include <stdio.h>

#include "pins.h"
#include "delay.h"

#include "../buzzer/buzzer.h"
#include "../comm/elevator_commands.h"
#include "../dotmatrix/dotmatrix.h"
#include "../fan/fan.h"
#include "../led/led.h"
#include "../test/test.h"

#define OBSTACLE_BLINK_INTERVAL_MS      800U
#define OBSTACLE_BLINK_TOGGLE_TOTAL     6U

typedef struct
{
	uint8_t current_command;
	uint8_t previous_command;
	uint8_t command_changed;
	uint8_t melody_enabled;
	uint16_t obstacle_elapsed_ms;
	uint8_t obstacle_toggle_count;
	uint8_t obstacle_led_is_on;
	uint8_t is_sleeping;
} elevator_runtime_context_t;

static elevator_runtime_context_t g_elevator_ctx = {
	CMD_IDLE,
	CMD_IDLE,
	1U,
	0U,
	0U,
	0U,
	0U,
	0U
};

static uint8_t elevator_is_runtime_command(uint8_t command)
{
	switch (command)
	{
		case CMD_IDLE:
		case CMD_MOVING_UP:
		case CMD_MOVING_DOWN:
		case CMD_DOOR_OPEN:
		case CMD_DOOR_CLOSE:
		case CMD_OBSTACLE:
		case CMD_FAULT:
			return 1U;

		default:
			return 0U;
	}
}

static void elevator_all_leds_off(void)
{
	led_off(LED_RED);
	led_off(LED_BLUE);
	led_off(LED_YELLOW);
	led_off(LED_GREEN);
}

static void elevator_apply_idle_outputs(void)
{
	elevator_all_leds_off();
	fan_off();
	dotmatrix_clear();
}

static void elevator_apply_moving_up_outputs(void)
{
	elevator_all_leds_off();
	led_on(LED_GREEN);
	fan_on();
	dotmatrix_show_up_arrow();
	buzzer_select_melody(BUZZER_MELODY_MOVING);
}

static void elevator_apply_moving_down_outputs(void)
{
	elevator_all_leds_off();
	led_on(LED_GREEN);
	fan_on();
	dotmatrix_show_down_arrow();
	buzzer_select_melody(BUZZER_MELODY_MOVING);
}

static void elevator_apply_door_open_outputs(void)
{
	elevator_all_leds_off();
	led_on(LED_BLUE);
	fan_off();
	dotmatrix_show_full_square_pattern();
	buzzer_select_melody(BUZZER_MELODY_DOOR);
}

static void elevator_apply_door_close_outputs(void)
{
	elevator_all_leds_off();
	led_on(LED_YELLOW);
	fan_off();
	dotmatrix_clear();
}

static void elevator_apply_fault_outputs(void)
{
	elevator_all_leds_off();
	led_on(LED_RED);
	fan_off();
	dotmatrix_show_checker_pattern();
}

static void elevator_obstacle_pattern_reset(void)
{
	g_elevator_ctx.obstacle_elapsed_ms = 0U;
	g_elevator_ctx.obstacle_toggle_count = 0U;
	g_elevator_ctx.obstacle_led_is_on = 1U;
	led_on(LED_RED);
}

static void elevator_apply_obstacle_outputs(void)
{
	elevator_all_leds_off();
	fan_off();
	dotmatrix_show_red_circle();
	elevator_obstacle_pattern_reset();
	buzzer_select_melody(BUZZER_MELODY_OBSTACLE);
}

static void elevator_handle_command_entry(void)
{
	switch (g_elevator_ctx.current_command)
	{
		case CMD_IDLE:
			elevator_apply_idle_outputs();
			break;

		case CMD_MOVING_UP:
			elevator_apply_moving_up_outputs();
			break;

		case CMD_MOVING_DOWN:
			elevator_apply_moving_down_outputs();
			break;

		case CMD_DOOR_OPEN:
			elevator_apply_door_open_outputs();
			break;

		case CMD_DOOR_CLOSE:
			elevator_apply_door_close_outputs();
			break;

		case CMD_OBSTACLE:
			elevator_apply_obstacle_outputs();
			break;

		case CMD_FAULT:
			elevator_apply_fault_outputs();
			break;

		default:
			elevator_apply_idle_outputs();
			break;
	}
}

static void elevator_task_obstacle_pattern(uint8_t tick_elapsed)
{
	if (g_elevator_ctx.obstacle_toggle_count >= OBSTACLE_BLINK_TOGGLE_TOTAL)
	{
		led_on(LED_RED);
		g_elevator_ctx.obstacle_led_is_on = 1U;
		return;
	}

	if (tick_elapsed == 0U)
	{
		return;
	}

	g_elevator_ctx.obstacle_elapsed_ms++;

	if (g_elevator_ctx.obstacle_elapsed_ms < OBSTACLE_BLINK_INTERVAL_MS)
	{
		return;
	}

	g_elevator_ctx.obstacle_elapsed_ms = 0U;

	if (g_elevator_ctx.obstacle_led_is_on != 0U)
	{
		led_off(LED_RED);
		g_elevator_ctx.obstacle_led_is_on = 0U;
	}
	else
	{
		led_on(LED_RED);
		g_elevator_ctx.obstacle_led_is_on = 1U;
	}

	g_elevator_ctx.obstacle_toggle_count++;

	if (g_elevator_ctx.obstacle_toggle_count >= OBSTACLE_BLINK_TOGGLE_TOTAL)
	{
		led_on(LED_RED);
		g_elevator_ctx.obstacle_led_is_on = 1U;
	}
}

static void elevator_apply_sleep_outputs(void)
{
	elevator_all_leds_off();
	led_all_on();
	fan_off();
	dotmatrix_clear();
	g_elevator_ctx.melody_enabled = 0U;
	buzzer_set_melody_loop(0U);
}

void elevator_enter_sleep(void)
{
	if (g_elevator_ctx.is_sleeping != 0U)
	{
		return;
	}

	g_elevator_ctx.is_sleeping = 1U;
	elevator_apply_sleep_outputs();
	printf("[ELEV] Entering sleep\r\n");
}

void elevator_exit_sleep(void)
{
	printf("[ELEV] Waking from sleep\r\n");
}

void elevator_init(void)
{
	g_elevator_ctx.current_command = CMD_IDLE;
	g_elevator_ctx.previous_command = CMD_IDLE;
	g_elevator_ctx.command_changed = 1U;
	g_elevator_ctx.melody_enabled = 0U;
	elevator_obstacle_pattern_reset();

	buzzer_set_melody_loop(0U);
	elevator_handle_command_entry();
	g_elevator_ctx.command_changed = 0U;
}

void elevator_set_command(uint8_t command)
{
	uint8_t requested_command = command;

	if (elevator_is_runtime_command(requested_command) == 0U)
	{
		requested_command = CMD_IDLE;
		printf("[ELEV] Unknown command 0x%02X -> IDLE\r\n", command);
	}

	if (requested_command != g_elevator_ctx.current_command)
	{
		g_elevator_ctx.previous_command = g_elevator_ctx.current_command;
		g_elevator_ctx.current_command = requested_command;
		g_elevator_ctx.command_changed = 1U;

		printf(
			"[ELEV] state 0x%02X -> 0x%02X\r\n",
			g_elevator_ctx.previous_command,
			g_elevator_ctx.current_command
		);
	}

	if (requested_command == CMD_IDLE)
	{
		g_elevator_ctx.melody_enabled = 0U;
	}

	if ((requested_command == CMD_MOVING_UP) || (requested_command == CMD_MOVING_DOWN))
	{
		g_elevator_ctx.melody_enabled = 1U;
	}

	if (requested_command == CMD_DOOR_OPEN)
	{
		g_elevator_ctx.melody_enabled = 1U;
	}

	if (requested_command == CMD_DOOR_CLOSE)
	{
		g_elevator_ctx.melody_enabled = 0U;
	}

	if (requested_command == CMD_OBSTACLE)
	{
		g_elevator_ctx.melody_enabled = 1U;
	}

	if (requested_command == CMD_FAULT)
	{
		g_elevator_ctx.melody_enabled = 0U;
	}
}

void elevator_set_melody_enabled(uint8_t enabled)
{
	uint8_t melody_state = (enabled != 0U) ? 1U : 0U;

	if (melody_state != g_elevator_ctx.melody_enabled)
	{
		g_elevator_ctx.melody_enabled = melody_state;
		printf("[ELEV] melody %s\r\n", (melody_state != 0U) ? "ON" : "OFF");
	}

	if (melody_state == 0U)
	{
		buzzer_set_melody_loop(0U);
	}
}

void elevator_task(void)
{
	uint8_t tick_elapsed = 0U;

	if (g_elevator_ctx.is_sleeping != 0U)
	{
		/* Power-down sleep: TWI address match wakes the MCU */
		SMCR = (1U << SM1) | (1U << SE);
		sleep_cpu();
		SMCR = 0U;
		/* Woken by TWI interrupt — turn off sleep indicator and let main loop continue */
		g_elevator_ctx.is_sleeping = 0U;
		led_all_off();
		g_elevator_ctx.previous_command = g_elevator_ctx.current_command;
		g_elevator_ctx.current_command = CMD_IDLE;
		g_elevator_ctx.command_changed = 1U;
		return;
	}

	if (g_elevator_ctx.command_changed != 0U)
	{
		elevator_handle_command_entry();
		g_elevator_ctx.command_changed = 0U;
	}

	if ((g_elevator_ctx.current_command == CMD_OBSTACLE) || (g_elevator_ctx.melody_enabled != 0U))
	{
		DELAY_ms(1);
		tick_elapsed = 1U;
	}

	if (g_elevator_ctx.current_command == CMD_OBSTACLE)
	{
		elevator_task_obstacle_pattern(tick_elapsed);
	}

	buzzer_set_melody_loop(g_elevator_ctx.melody_enabled);

	if (tick_elapsed != 0U)
	{
		buzzer_task();
	}
}

uint8_t elevator_get_current_command(void)
{
	return g_elevator_ctx.current_command;
}
