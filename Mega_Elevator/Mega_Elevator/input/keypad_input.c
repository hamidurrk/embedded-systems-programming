/*
 * keypad_input.c
 *
 * Created: 4/11/2026 1:36:54 AM
 *  Author: Hamidur
 */ 

#include "keypad_input.h"

#include "keypad.h"

static uint16_t s_floor_accumulator = 0u;
static uint8_t s_has_digits = 0u;
static uint8_t s_floor_invalid = 0u;

static uint8_t keypad_input_process_key(uint8_t key_signal, uint8_t *floor_out, uint8_t report_invalid_confirm_as_pending);

void keypad_input_init(void)
{
	KEYPAD_Init();
	s_floor_accumulator = 0u;
	s_has_digits = 0u;
	s_floor_invalid = 0u;
}

void keypad_input_reset_accumulator(void)
{
	s_floor_accumulator = 0u;
	s_has_digits = 0u;
	s_floor_invalid = 0u;
}

const char *keypad_input_get_key(void)
{
	static char key_str[2];
	uint8_t key_signal = KEYPAD_GetKey();

	key_str[0] = (char)key_signal;
	key_str[1] = '\0';
	return key_str;
}

uint8_t keypad_input_get_key_if_pressed(uint8_t *key_out)
{
	uint8_t key_signal;

	if (key_out == (uint8_t *)0)
	{
		return 0u;
	}

	key_signal = KEYPAD_GetKeyIfPressed();
	if (key_signal == '\0')
	{
		return 0u;
	}

	*key_out = key_signal;
	return 1u;
}

uint8_t keypad_input_get_key_nonblocking(uint8_t *key_out)
{
	uint8_t key_signal;

	if (key_out == (uint8_t *)0)
	{
		return 0u;
	}

	key_signal = KEYPAD_GetKeyNonBlocking();
	if (key_signal == '\0')
	{
		return 0u;
	}

	*key_out = key_signal;
	return 1u;
}

uint8_t keypad_input_read_floor(uint8_t *floor_out)
{
	if (floor_out == (uint8_t *)0)
	{
		return 0u;
	}

	return keypad_input_process_key(KEYPAD_GetKey(), floor_out, 0u);
}

uint8_t keypad_input_read_floor_nonblocking(uint8_t *floor_out)
{
	uint8_t key_signal;

	if (floor_out == (uint8_t *)0)
	{
		return KEYPAD_INPUT_READ_NONE;
	}

	if (keypad_input_get_key_nonblocking(&key_signal) == 0u)
	{
		return KEYPAD_INPUT_READ_NONE;
	}

	return keypad_input_process_key(key_signal, floor_out, 1u);
}

static uint8_t keypad_input_process_key(uint8_t key_signal, uint8_t *floor_out, uint8_t report_invalid_confirm_as_pending)
{
	uint8_t digit;
	uint16_t next_value;

	if (key_signal == '*')
	{
		s_floor_accumulator = 0u;
		s_has_digits = 0u;
		s_floor_invalid = 0u;
		*floor_out = key_signal;
		return KEYPAD_INPUT_READ_PENDING;
	}

	if (key_signal == '#')
	{
		if ((s_has_digits != 0u) && (s_floor_invalid == 0u) && (s_floor_accumulator <= 99u))
		{
			*floor_out = (uint8_t)s_floor_accumulator;
			s_floor_accumulator = 0u;
			s_has_digits = 0u;
			s_floor_invalid = 0u;
			return KEYPAD_INPUT_READ_CONFIRMED;
		}

		s_floor_accumulator = 0u;
		s_has_digits = 0u;
		s_floor_invalid = 0u;
		if (report_invalid_confirm_as_pending != 0u)
		{
			*floor_out = key_signal;
			return KEYPAD_INPUT_READ_PENDING;
		}

		return KEYPAD_INPUT_READ_NONE;
	}

	if ((key_signal >= '0') && (key_signal <= '9'))
	{
		digit = (uint8_t)(key_signal - '0');
		next_value = (uint16_t)((s_floor_accumulator * 10u) + digit);

		s_floor_accumulator = next_value;
		s_has_digits = 1u;
		if (next_value > 99u)
		{
			s_floor_invalid = 1u;
		}

		*floor_out = key_signal;
		return KEYPAD_INPUT_READ_PENDING;
	}

	*floor_out = key_signal;
	return KEYPAD_INPUT_READ_PENDING;
}

const char *keypad_input_floor_to_display(uint8_t read_status, uint8_t floor_out)
{
	static char display_str[4];

	if (read_status == 1u)
	{
		if (floor_out > 99u)
		{
			display_str[0] = '\0';
			return display_str;
		}

		if (floor_out >= 10u)
		{
			display_str[0] = (char)('0' + (floor_out / 10u));
			display_str[1] = (char)('0' + (floor_out % 10u));
			display_str[2] = '\0';
			return display_str;
		}

		display_str[0] = (char)('0' + floor_out);
		display_str[1] = '\0';
		return display_str;
	}

	if (read_status == 2u)
	{
		display_str[0] = (char)floor_out;
		display_str[1] = '\0';
		return display_str;
	}

	display_str[0] = '\0';
	return display_str;
}
