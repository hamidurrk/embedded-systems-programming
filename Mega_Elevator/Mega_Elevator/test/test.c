/*
 * test.c
 *
 * Created: 4/12/2026 7:06:29 AM
 *  Author: Hamidur
 */ 

#include "test.h"

#include <stdio.h>

#include "../comm/elevator_commands.h"
#include "../comm/i2c_master.h"
#include "../display/display.h"
#include "../input/keypad_input.h"
#include "../sonar/sonar.h"

#include "delay.h"

static char s_input_code[3] = {'\0', '\0', '\0'};
static uint8_t s_input_len = 0u;
static uint8_t s_sonar_distance_test_initialized = 0u;
static uint8_t s_sonar_stream_active = 0u;

static void sonar_test_stream_start(void)
{
	s_sonar_distance_test_initialized = 0u;
	s_sonar_stream_active = 1u;
	display_clear();
	display_write("SONAR STREAM");
}

static void sonar_test_stream_stop(void)
{
	s_sonar_stream_active = 0u;
	s_sonar_distance_test_initialized = 0u;
	display_clear();
	display_write("SONAR STOP");
}

static void sonar_test_distance(void)
{
	uint16_t distance_cm;
	char lcd_text[17];

	if (s_sonar_distance_test_initialized == 0u)
	{
		sonar_init();
		printf("Sonar distance stream started.\r\n");
		s_sonar_distance_test_initialized = 1u;
	}

	distance_cm = sonar_measure_distance_cm();

	if (distance_cm != SONAR_INVALID_DISTANCE_CM)
	{
		printf("Distance: %u cm\r\n", distance_cm);
		(void)snprintf(lcd_text, sizeof(lcd_text), "Dist: %u cm", distance_cm);
		display_clear();
		display_write(lcd_text);
	}
	else
	{
		printf("Distance: invalid\r\n");
		display_clear();
		display_write("Dist: invalid");
	}

	DELAY_ms(250u);
}

static void test_reset_input(void)
{
	s_input_code[0] = '\0';
	s_input_code[1] = '\0';
	s_input_code[2] = '\0';
	s_input_len = 0u;
}

static void test_show_input(void)
{
	display_clear();
	display_write(s_input_code);
}

static uint8_t test_map_code_to_command(const char *code, uint8_t *command_out)
{
	if ((code == (const char *)0) || (command_out == (uint8_t *)0))
	{
		return 0u;
	}

	if ((code[0] != 'A') || (code[2] != '\0'))
	{
		return 0u;
	}

	switch (code[1])
	{
	case '1':
		*command_out = CMD_TEST_LED;
		return 1u;
	case '2':
		*command_out = CMD_TEST_BUZZER;
		return 1u;
	case '3':
		*command_out = CMD_TEST_FAN;
		return 1u;
	case '4':
		*command_out = CMD_TEST_SONAR;
		return 1u;
	case '5':
		*command_out = CMD_TEST_DOTMATRIX;
		return 1u;
	default:
		return 0u;
	}
}

void test_main(void)
{
	const char *key_str;
	char key;
	uint8_t command;
	char ok_msg[6];
	uint8_t stream_key;

	if (s_sonar_stream_active != 0u)
	{
		if ((keypad_input_get_key_if_pressed(&stream_key) != 0u) && (stream_key == '*'))
		{
			/* Exit sonar stream mode when '*' is pressed. */
			sonar_test_stream_stop();
			test_reset_input();
			return;
		}

		sonar_test_distance();
		return;
	}

	key_str = keypad_input_get_key();

	if ((key_str == (const char *)0) || (key_str[0] == '\0'))
	{
		return;
	}

	key = key_str[0];

	if (key == '*')
	{
		s_sonar_distance_test_initialized = 0u;
		s_sonar_stream_active = 0u;
		test_reset_input();
		display_clear();
		return;
	}

	if (key == '#')
	{
		if ((s_input_len == 2u) && (test_map_code_to_command(s_input_code, &command) != 0u))
		{
			if (command == CMD_TEST_SONAR)
			{
				sonar_test_stream_start();
			}
			else
			{
				i2c_master_send_command(command);

				ok_msg[0] = s_input_code[0];
				ok_msg[1] = s_input_code[1];
				ok_msg[2] = ' ';
				ok_msg[3] = 'O';
				ok_msg[4] = 'K';
				ok_msg[5] = '\0';

				display_clear();
				display_write(ok_msg);
			}
		}
		else
		{
			display_clear();
			display_write("INVALID");
		}

		test_reset_input();
		return;
	}

	if (key == 'A')
	{
		if (s_input_len == 0u)
		{
			s_input_code[0] = 'A';
			s_input_code[1] = '\0';
			s_input_len = 1u;
			test_show_input();
		}
		else
		{
			display_clear();
			display_write("INVALID");
			test_reset_input();
		}
		return;
	}

	if ((key >= '1') && (key <= '5'))
	{
		if ((s_input_len == 1u) && (s_input_code[0] == 'A'))
		{
			s_input_code[1] = key;
			s_input_code[2] = '\0';
			s_input_len = 2u;
			test_show_input();
		}
		else
		{
			display_clear();
			display_write("INVALID");
			test_reset_input();
		}
		return;
	}

	display_clear();
	display_write("INVALID");
	test_reset_input();
}
