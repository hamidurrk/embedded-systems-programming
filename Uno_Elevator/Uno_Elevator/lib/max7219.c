/*
 * max7219.c
 *
 * Created: 4/12/2026 5:44:21 AM
 *  Author: Hamidur
 */ 

#define F_CPU 16000000UL

#include "max7219.h"
#include "font5x7.h"

#include <avr/io.h>
#include <util/delay.h>

// Fixed wiring:
// DIN -> D11 -> PB3 -> MOSI
// CLK -> D13 -> PB5 -> SCK
// CS  -> D12 -> PB4  (manual chip select)

#define MAX7219_DDR   DDRB
#define MAX7219_PORT  PORTB

#define DIN_PIN   PB3
#define CLK_PIN   PB5
#define CS_PIN    PB4

#define MAX7219_REG_NOOP         0x00U
#define MAX7219_REG_DIGIT0       0x01U
#define MAX7219_REG_DECODE_MODE  0x09U
#define MAX7219_REG_INTENSITY    0x0AU
#define MAX7219_REG_SCAN_LIMIT   0x0BU
#define MAX7219_REG_SHUTDOWN     0x0CU
#define MAX7219_REG_DISPLAY_TEST 0x0FU

#define MAX7219_STARTUP_DELAY_MS         100U
#define MAX7219_BOOT_PATTERN_HOLD_MS     300U
#define MAX7219_BOOT_DISPLAY_TEST_MS     200U
#define MAX7219_DEFAULT_INTENSITY        0x08U
#define MAX7219_ENABLE_BOOT_SELF_TEST    0U

static void max7219_bus_init(void)
{
	/* Bit-banged serial: DIN, CLK and CS as GPIO outputs. */
	MAX7219_DDR |= (1 << DIN_PIN) | (1 << CLK_PIN) | (1 << CS_PIN);

	/* Idle levels */
	MAX7219_PORT &= ~(1 << CLK_PIN);
	MAX7219_PORT &= ~(1 << DIN_PIN);
	MAX7219_PORT |= (1 << CS_PIN);
}

static void spi_send(uint8_t data)
{
	for (uint8_t mask = 0x80U; mask != 0U; mask >>= 1U)
	{
		if ((data & mask) != 0U)
		{
			MAX7219_PORT |= (1 << DIN_PIN);
		}
		else
		{
			MAX7219_PORT &= ~(1 << DIN_PIN);
		}

		MAX7219_PORT |= (1 << CLK_PIN);
		_delay_us(1);
		MAX7219_PORT &= ~(1 << CLK_PIN);
		_delay_us(1);
	}
}

static void max7219_send(uint8_t reg, uint8_t data)
{
	MAX7219_PORT &= ~(1 << CS_PIN);
	spi_send(reg);
	spi_send(data);
	MAX7219_PORT |= (1 << CS_PIN);
}

#if (MAX7219_ENABLE_BOOT_SELF_TEST == 1U)
static void max7219_show_boot_pattern(void)
{
	for (uint8_t row = 0U; row < 8U; row++)
	{
		uint8_t row_data = (row & 1U) ? 0xAAU : 0x55U;
		max7219_set_row(row, row_data);
	}

	_delay_ms(MAX7219_BOOT_PATTERN_HOLD_MS);
	max7219_clear();
}
#endif

void max7219_init(void)
{
	max7219_bus_init();
	_delay_ms(MAX7219_STARTUP_DELAY_MS);

	max7219_send(MAX7219_REG_DISPLAY_TEST, 0x00U); // display test off
	max7219_send(MAX7219_REG_SHUTDOWN, 0x01U);     // normal operation
	max7219_send(MAX7219_REG_SCAN_LIMIT, 0x07U);   // scan limit = 8 rows
	max7219_send(MAX7219_REG_DECODE_MODE, 0x00U);  // no decode
	max7219_send(MAX7219_REG_INTENSITY, MAX7219_DEFAULT_INTENSITY);
	max7219_clear();

#if (MAX7219_ENABLE_BOOT_SELF_TEST == 1U)
	max7219_send(MAX7219_REG_DISPLAY_TEST, 0x01U);
	_delay_ms(MAX7219_BOOT_DISPLAY_TEST_MS);
	max7219_send(MAX7219_REG_DISPLAY_TEST, 0x00U);

	max7219_show_boot_pattern();
#endif
}

void max7219_clear(void)
{
	for (uint8_t i = 1; i <= 8; i++)
	{
		max7219_send(i, 0x00);
	}
}

void max7219_set_row(uint8_t row, uint8_t value)
{
	if (row < 8)
	{
		max7219_send(row + 1, value);
	}
}

void max7219_clear_buffer(bool buffer[8][8])
{
	for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			buffer[y][x] = false;
		}
	}
}

void max7219_draw_char(bool buffer[8][8], int xOffset, char c)
{
	const uint8_t* bmp = font5x7_get_char(c);

	for (uint8_t y = 0; y < 7; y++)
	{
		for (uint8_t x = 0; x < 5; x++)
		{
			bool pixelOn = (bmp[y] & (1 << (4 - x))) != 0;
			int px = xOffset + x;
			int py = y;

			if (px >= 0 && px < 8 && py >= 0 && py < 8)
			{
				buffer[py][px] = pixelOn;
			}
		}
	}
}

void max7219_show_rotated_clockwise(bool buffer[8][8])
{
	for (uint8_t row = 0; row < 8; row++)
	{
		uint8_t rowData = 0;

		for (uint8_t col = 0; col < 8; col++)
		{
			// physical(row, col) = logical(7-col, row)
			if (buffer[7 - col][row])
			{
				rowData |= (1 << (7 - col));
			}
		}

		max7219_set_row(row, rowData);
	}
}