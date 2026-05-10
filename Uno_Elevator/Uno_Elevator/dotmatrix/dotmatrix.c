/*
 * dotmatrix.c
 *
 * Created: 4/12/2026 3:32:06 AM
 *  Author: Hamidur
 */

#include "dotmatrix.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "max7219.h"
#include "delay.h"

static const uint8_t DOTMATRIX_UP_ARROW[8] = {
	0x18U, 0x3CU, 0x7EU, 0x18U, 0x18U, 0x18U, 0x18U, 0x18U
};

static const uint8_t DOTMATRIX_DOWN_ARROW[8] = {
	0x18U, 0x18U, 0x18U, 0x18U, 0x18U, 0x7EU, 0x3CU, 0x18U
};

static const uint8_t DOTMATRIX_RED_CIRCLE[8] = {
	0x3CU, 0x42U, 0x81U, 0x81U, 0x81U, 0x81U, 0x42U, 0x3CU
};

static const uint8_t DOTMATRIX_CHECKER[8] = {
	0xAAU, 0x55U, 0xAAU, 0x55U, 0xAAU, 0x55U, 0xAAU, 0x55U
};

static const uint8_t DOTMATRIX_FULL_SQUARE[8] = {
	0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU
};

static void dotmatrix_show_rows(const uint8_t rows[8])
{
	bool buffer[8][8];
	uint8_t y;
	uint8_t x;

	max7219_clear_buffer(buffer);

	for (y = 0U; y < 8U; y++)
	{
		for (x = 0U; x < 8U; x++)
		{
			buffer[y][x] = ((rows[y] & (uint8_t)(1U << (7U - x))) != 0U);
		}
	}

	max7219_show_rotated_clockwise(buffer);
}

void dotmatrix_init(void)
{
    max7219_init();
    max7219_clear();
}

void dotmatrix_clear(void)
{
	max7219_clear();
}

void dotmatrix_show_up_arrow(void)
{
	dotmatrix_show_rows(DOTMATRIX_UP_ARROW);
}

void dotmatrix_show_down_arrow(void)
{
	dotmatrix_show_rows(DOTMATRIX_DOWN_ARROW);
}

void dotmatrix_show_red_circle(void)
{
	dotmatrix_show_rows(DOTMATRIX_RED_CIRCLE);
}

void dotmatrix_show_checker_pattern(void)
{
	dotmatrix_show_rows(DOTMATRIX_CHECKER);
}

void dotmatrix_show_full_square_pattern(void)
{
	dotmatrix_show_rows(DOTMATRIX_FULL_SQUARE);
}

void dotmatrix_print_message(const char *message)
{
	bool buffer[8][8];
	int msg_len;
	int total_width;
	int scroll;
	int i;

	if ((message == 0) || (message[0] == '\0'))
	{
		return;
	}

	msg_len = (int)strlen(message);
	total_width = msg_len * 6; /* 5 pixels + 1 spacing column */

	while (1)
	{
		for (scroll = 8; scroll > -total_width; scroll--)
		{
			max7219_clear_buffer(buffer);

			for (i = 0; i < msg_len; i++)
			{
				int char_x = scroll + (i * 6);
				max7219_draw_char(buffer, char_x, message[i]);
			}

			max7219_show_rotated_clockwise(buffer);
			DELAY_ms(120);
		}
	}
}
