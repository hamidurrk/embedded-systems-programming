/*
 * elevator.c
 *
 * Created: 4/11/2026 1:35:36 AM
 *  Author: Hamidur
 */ 

#include "elevator.h"

#include "delay.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>

#include "../comm/elevator_commands.h"
#include "../comm/i2c_master.h"
#include "../config/config.h"
#include "../config/timing.h"
#include "../display/display.h"
#include "../input/keypad_input.h"
#include "keypad.h"
#include "../sonar/sonar.h"

#define ELEVATOR_IDLE_INPUT_MAX_CHARS (3u)

static ElevatorState s_state = STATE_IDLE;
static uint8_t s_state_entry_pending = 1u;
static uint16_t s_state_elapsed_ms = 0u;

static uint8_t s_current_floor = ELEVATOR_START_FLOOR;
static uint8_t s_target_floor = ELEVATOR_START_FLOOR;
static uint8_t s_has_target_floor = 0u;

static uint8_t s_floor_queue[ELEVATOR_QUEUE_CAPACITY];
static uint8_t s_queue_head = 0u;
static uint8_t s_queue_tail = 0u;
static uint8_t s_queue_count = 0u;
static uint16_t s_sonar_sample_elapsed_ms = 0u;
static uint8_t s_obstacle_below_threshold_streak = 0u;
static char s_idle_input[ELEVATOR_IDLE_INPUT_MAX_CHARS + 1u] = {'\0', '\0', '\0', '\0'};
static uint8_t s_idle_input_len = 0u;
static uint16_t s_sleep_key_hold_ms = 0u;
static uint8_t s_sleep_armed = 0u;

static void elevator_enter_state(ElevatorState new_state);
static void elevator_on_state_entry(void);
static void elevator_start_next_request_or_idle(void);
static void elevator_submit_floor_request(uint8_t floor);

static uint8_t elevator_queue_contains(uint8_t floor);
static uint8_t elevator_queue_enqueue(uint8_t floor);
static uint8_t elevator_queue_dequeue(uint8_t *floor_out);
static void elevator_reset_closing_obstacle_tracking(void);
static uint8_t elevator_should_trigger_closing_obstacle(void);
static void elevator_idle_input_reset(void);
static void elevator_idle_input_render(void);
static void elevator_idle_input_handle_pending_key(uint8_t key_signal);
static void elevator_sleep_handle_idle_trigger(void);
static void elevator_enter_sleep_mode(void);

void elevator_init(void)
{
	s_state = STATE_IDLE;
	s_state_entry_pending = 1u;
	s_state_elapsed_ms = 0u;

	s_current_floor = ELEVATOR_START_FLOOR;
	s_target_floor = ELEVATOR_START_FLOOR;
	s_has_target_floor = 0u;

	s_queue_head = 0u;
	s_queue_tail = 0u;
	s_queue_count = 0u;
	s_sonar_sample_elapsed_ms = 0u;
	s_obstacle_below_threshold_streak = 0u;
	s_sleep_key_hold_ms = 0u;
	s_sleep_armed = 0u;
	elevator_idle_input_reset();
}

void elevator_task(void)
{
	uint8_t input_data = 0u;
	uint8_t input_status;
	uint8_t any_key_pressed = 0u;
	uint8_t obstacle_key_pressed = 0u;
	ElevatorState state_before;

	input_status = keypad_input_read_floor_nonblocking(&input_data);
	if (input_status != KEYPAD_INPUT_READ_NONE)
	{
		any_key_pressed = 1u;
		if ((input_status == KEYPAD_INPUT_READ_PENDING) && (input_data == (uint8_t)ELEVATOR_OBSTACLE_KEY))
		{
			obstacle_key_pressed = 1u;
		}

		if (input_status == KEYPAD_INPUT_READ_CONFIRMED)
		{
			elevator_submit_floor_request(input_data);
		}
	}

	if (s_state_entry_pending != 0u)
	{
		elevator_on_state_entry();
		s_state_entry_pending = 0u;
	}

	state_before = s_state;

	switch (s_state)
	{
	case STATE_IDLE:
		if (input_status == KEYPAD_INPUT_READ_PENDING)
		{
			elevator_idle_input_handle_pending_key(input_data);
		}
		else if (input_status == KEYPAD_INPUT_READ_CONFIRMED)
		{
			elevator_idle_input_reset();
		}

		elevator_start_next_request_or_idle();
		if (s_state == STATE_IDLE)
		{
			elevator_sleep_handle_idle_trigger();
		}
		break;

	case STATE_GOING_UP:
		if (s_state_elapsed_ms >= ELEVATOR_MOVE_ONE_FLOOR_MS)
		{
			s_state_elapsed_ms = 0u;
			if (s_current_floor < ELEVATOR_MAX_FLOOR)
			{
				s_current_floor++;
			}

			display_show_floor(s_current_floor);
			if (s_current_floor >= s_target_floor)
			{
				s_has_target_floor = 0u;
				elevator_enter_state(STATE_DOOR_OPENING);
			}
		}
		break;

	case STATE_GOING_DOWN:
		if (s_state_elapsed_ms >= ELEVATOR_MOVE_ONE_FLOOR_MS)
		{
			s_state_elapsed_ms = 0u;
			if (s_current_floor > 0u)
			{
				s_current_floor--;
			}

			display_show_floor(s_current_floor);
			if (s_current_floor <= s_target_floor)
			{
				s_has_target_floor = 0u;
				elevator_enter_state(STATE_DOOR_OPENING);
			}
		}
		break;

	case STATE_DOOR_OPENING:
		if (s_state_elapsed_ms >= ELEVATOR_DOOR_OPEN_MS)
		{
			elevator_enter_state(STATE_DOOR_CLOSING);
		}
		break;

	case STATE_OBSTACLE_DETECTION:
		if (any_key_pressed != 0u)
		{
			i2c_master_send_command(CMD_STOP_MELODY);
			elevator_enter_state(STATE_DOOR_CLOSING);
		}
		break;

	case STATE_DOOR_CLOSING:
		if ((ELEVATOR_OBSTACLE_FALLBACK_ENABLED != 0u) && (obstacle_key_pressed != 0u))
		{
			elevator_enter_state(STATE_OBSTACLE_DETECTION);
		}
		else if (elevator_should_trigger_closing_obstacle() != 0u)
		{
			elevator_enter_state(STATE_OBSTACLE_DETECTION);
		}
		else if (s_state_elapsed_ms >= ELEVATOR_DOOR_CLOSE_MS)
		{
			elevator_start_next_request_or_idle();
		}
		break;

	case STATE_FAULT:
		if (s_state_elapsed_ms >= ELEVATOR_FAULT_MS)
		{
			elevator_enter_state(STATE_IDLE);
		}
		break;

	default:
		elevator_enter_state(STATE_IDLE);
		break;
	}

	if (s_state != STATE_IDLE)
	{
		s_sleep_key_hold_ms = 0u;
		s_sleep_armed = 0u;
	}

	if (state_before == s_state)
	{
		if (s_state_elapsed_ms <= (uint16_t)(0xFFFFu - APP_LOOP_TICK_MS))
		{
			s_state_elapsed_ms = (uint16_t)(s_state_elapsed_ms + APP_LOOP_TICK_MS);
		}

		if (s_state == STATE_DOOR_CLOSING)
		{
			if (s_sonar_sample_elapsed_ms <= (uint16_t)(0xFFFFu - APP_LOOP_TICK_MS))
			{
				s_sonar_sample_elapsed_ms = (uint16_t)(s_sonar_sample_elapsed_ms + APP_LOOP_TICK_MS);
			}
		}
	}

	DELAY_ms(APP_LOOP_TICK_MS);
}

static void elevator_enter_state(ElevatorState new_state)
{
	s_state = new_state;
	s_state_entry_pending = 1u;
	s_state_elapsed_ms = 0u;
}

static void elevator_on_state_entry(void)
{
	switch (s_state)
	{
	case STATE_IDLE:
		i2c_master_send_command(CMD_IDLE);
		keypad_input_reset_accumulator();
		elevator_idle_input_reset();
		elevator_idle_input_render();
		break;

	case STATE_GOING_UP:
		i2c_master_send_command(CMD_MOVING_UP);
		display_show_floor(s_current_floor);
		break;

	case STATE_GOING_DOWN:
		i2c_master_send_command(CMD_MOVING_DOWN);
		display_show_floor(s_current_floor);
		break;

	case STATE_DOOR_OPENING:
		i2c_master_send_command(CMD_DOOR_OPEN);
		display_show_door_open();
		break;

	case STATE_OBSTACLE_DETECTION:
		i2c_master_send_command(CMD_OBSTACLE);
		i2c_master_send_command(CMD_PLAY_MELODY);
		display_show_obstacle();
		break;

	case STATE_DOOR_CLOSING:
		i2c_master_send_command(CMD_DOOR_CLOSE);
		display_show_door_closing();
		elevator_reset_closing_obstacle_tracking();
		break;

	case STATE_FAULT:
		i2c_master_send_command(CMD_FAULT);
		display_show_same_floor_fault();
		break;

	default:
		break;
	}
}

static void elevator_reset_closing_obstacle_tracking(void)
{
	/* Force first sonar check immediately after entering DOOR_CLOSING. */
	s_sonar_sample_elapsed_ms = ELEVATOR_SONAR_SAMPLE_MS;
	s_obstacle_below_threshold_streak = 0u;
}

static void elevator_idle_input_reset(void)
{
	s_idle_input_len = 0u;
	s_idle_input[0] = '\0';
}

static void elevator_idle_input_render(void)
{
	display_show_idle_input(s_idle_input);
}

static void elevator_idle_input_handle_pending_key(uint8_t key_signal)
{
	if ((key_signal == (uint8_t)'*') || (key_signal == (uint8_t)'#'))
	{
		elevator_idle_input_reset();
		elevator_idle_input_render();
		return;
	}

	if ((key_signal >= (uint8_t)'0') && (key_signal <= (uint8_t)'9'))
	{
		if (s_idle_input_len < ELEVATOR_IDLE_INPUT_MAX_CHARS)
		{
			s_idle_input[s_idle_input_len] = (char)key_signal;
			s_idle_input_len++;
			s_idle_input[s_idle_input_len] = '\0';
		}

		elevator_idle_input_render();
	}
}

static void elevator_sleep_handle_idle_trigger(void)
{
	uint8_t key_signal = '\0';

	if (ELEVATOR_SLEEP_MODE_ENABLED == 0u)
	{
		return;
	}

	if ((keypad_input_get_key_if_pressed(&key_signal) != 0u) && (key_signal == (uint8_t)ELEVATOR_SLEEP_TRIGGER_KEY))
	{
		if (s_sleep_key_hold_ms <= (uint16_t)(0xFFFFu - APP_LOOP_TICK_MS))
		{
			s_sleep_key_hold_ms = (uint16_t)(s_sleep_key_hold_ms + APP_LOOP_TICK_MS);
		}

		if (s_sleep_key_hold_ms >= ELEVATOR_SLEEP_TRIGGER_HOLD_MS)
		{
			s_sleep_armed = 1u;
		}
		return;
	}

	if (s_sleep_armed != 0u)
	{
		s_sleep_armed = 0u;
		s_sleep_key_hold_ms = 0u;
		elevator_enter_sleep_mode();
		return;
	}

	s_sleep_key_hold_ms = 0u;
}

static void elevator_enter_sleep_mode(void)
{
	uint8_t prr0_before;
	uint8_t prr1_before;
	uint8_t wake_key_seen = 0u;
	uint8_t pressed_key = '\0';

	i2c_master_send_command(CMD_SLEEP);

	display_clear();
	display_write("Sleep mode");
	DELAY_ms(50u);

	KEYPAD_ClearWakeFlag();
	KEYPAD_EnableWakeInterrupt();

	prr0_before = PRR0;
	prr1_before = PRR1;

	/* Disable peripherals while sleeping to reduce current draw. */
	PRR0 = (uint8_t)(PRR0 | (1u << PRADC) | (1u << PRSPI) | (1u << PRTIM1) |
		(1u << PRTIM2) | (1u << PRTIM0) | (1u << PRUSART0) | (1u << PRTWI));
	PRR1 = (uint8_t)(PRR1 | (1u << PRTIM3) | (1u << PRTIM4) | (1u << PRTIM5) |
		(1u << PRUSART1) | (1u << PRUSART2) | (1u << PRUSART3));

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();

	PRR0 = prr0_before;
	PRR1 = prr1_before;
	i2c_master_reinit();
	i2c_master_send_command(CMD_WAKE);

	/* Consume wake key until it is released so no phantom input is processed. */
	do
	{
		if (keypad_input_get_key_if_pressed(&pressed_key) != 0u)
		{
			wake_key_seen = 1u;
		}
		else if ((wake_key_seen != 0u) || (KEYPAD_GetWakeFlag() != 0u))
		{
			break;
		}
	} while (1);

	KEYPAD_DisableWakeInterrupt();
	KEYPAD_ClearWakeFlag();
	KEYPAD_Init();

	elevator_idle_input_reset();
	elevator_idle_input_render();
}

static uint8_t elevator_should_trigger_closing_obstacle(void)
{
	uint16_t distance_cm;

	if (s_sonar_sample_elapsed_ms < ELEVATOR_SONAR_SAMPLE_MS)
	{
		return 0u;
	}

	s_sonar_sample_elapsed_ms = 0u;
	distance_cm = sonar_measure_distance_cm();

	if (distance_cm == SONAR_INVALID_DISTANCE_CM)
	{
		/* Invalid samples do not change the current streak. */
		return 0u;
	}

	if (distance_cm < ELEVATOR_OBSTACLE_THRESHOLD_CM)
	{
		if (s_obstacle_below_threshold_streak < 0xFFu)
		{
			s_obstacle_below_threshold_streak++;
		}

		if (s_obstacle_below_threshold_streak >= ELEVATOR_OBSTACLE_STREAK_REQUIRED)
		{
			return 1u;
		}

		return 0u;
	}

	s_obstacle_below_threshold_streak = 0u;
	return 0u;
}

static void elevator_start_next_request_or_idle(void)
{
	uint8_t next_floor;

	if (elevator_queue_dequeue(&next_floor) == 0u)
	{
		s_has_target_floor = 0u;
		if (s_state != STATE_IDLE)
		{
			elevator_enter_state(STATE_IDLE);
		}
		return;
	}

	s_target_floor = next_floor;
	s_has_target_floor = 1u;

	printf("[ELEV] next=%u cur=%u\r\n", s_target_floor, s_current_floor);

	if (s_target_floor == s_current_floor)
	{
		s_has_target_floor = 0u;
		elevator_enter_state(STATE_FAULT);
		return;
	}

	if (s_target_floor > s_current_floor)
	{
		elevator_enter_state(STATE_GOING_UP);
	}
	else
	{
		elevator_enter_state(STATE_GOING_DOWN);
	}
}

static void elevator_submit_floor_request(uint8_t floor)
{
	if (floor > ELEVATOR_MAX_FLOOR)
	{
		printf("[REQ] rejected: floor %u > max\r\n", floor);
		return;
	}

	if ((s_state == STATE_IDLE) && (floor == s_current_floor))
	{
		printf("[REQ] fault: idle at floor %u\r\n", floor);
		elevator_enter_state(STATE_FAULT);
		return;
	}

	if ((s_has_target_floor != 0u) && (floor == s_target_floor))
	{
		printf("[REQ] dropped: == target %u\r\n", floor);
		return;
	}

	if ((s_state != STATE_IDLE) && (floor == s_current_floor))
	{
		printf("[REQ] dropped: == current %u\r\n", floor);
		return;
	}

	if (elevator_queue_contains(floor) != 0u)
	{
		printf("[REQ] dropped: already queued %u\r\n", floor);
		return;
	}

	printf("[REQ] enqueued floor %u (cur=%u target=%u htf=%u)\r\n",
		floor, s_current_floor, s_target_floor, s_has_target_floor);
	(void)elevator_queue_enqueue(floor);
}

static uint8_t elevator_queue_contains(uint8_t floor)
{
	uint8_t idx;
	uint8_t queue_index;

	for (idx = 0u; idx < s_queue_count; idx++)
	{
		queue_index = (uint8_t)((s_queue_head + idx) % ELEVATOR_QUEUE_CAPACITY);
		if (s_floor_queue[queue_index] == floor)
		{
			return 1u;
		}
	}

	return 0u;
}

static uint8_t elevator_queue_enqueue(uint8_t floor)
{
	if (s_queue_count >= ELEVATOR_QUEUE_CAPACITY)
	{
		return 0u;
	}

	s_floor_queue[s_queue_tail] = floor;
	s_queue_tail = (uint8_t)((s_queue_tail + 1u) % ELEVATOR_QUEUE_CAPACITY);
	s_queue_count++;
	return 1u;
}

static uint8_t elevator_queue_dequeue(uint8_t *floor_out)
{
	if ((floor_out == (uint8_t *)0) || (s_queue_count == 0u))
	{
		return 0u;
	}

	*floor_out = s_floor_queue[s_queue_head];
	s_queue_head = (uint8_t)((s_queue_head + 1u) % ELEVATOR_QUEUE_CAPACITY);
	s_queue_count--;
	return 1u;
}
