/*
 * display.c
 *
 * Created: 4/11/2026 1:37:24 AM
 *  Author: Hamidur
 */ 

#include "lcd.h"
#include "display.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define LCD_MAX_STRING (32)

static uint8_t s_display_chars = 0u;

static void display_show_two_lines(const char *line_1, const char *line_2);

void display_init(void) {
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
	s_display_chars = 0u;
	printf("LCD Ready.\r\n");
}

void display_write(const char* string) {
	uint8_t len = strnlen(string, LCD_MAX_STRING);
	if (LCD_MAX_STRING == len) {
		printf("Failed to print LCD string. Too big or lacks NULL-terminator.\r\n");
		for (uint8_t i = 0; i < len; i++)
		{
		    printf("%c", string[i]);
		}
		printf("\r\n");
	} else {
	    printf("LCD output: '%s'\r\n", string);
	    lcd_puts(string);
		s_display_chars = (uint8_t)((s_display_chars + len) > LCD_MAX_STRING ? LCD_MAX_STRING : (s_display_chars + len));
	}

}

void display_delete_last(void) {
	if (s_display_chars == 0u) {
		return;
	}

	lcd_command(LCD_MOVE_CURSOR_LEFT);
	lcd_putc(' ');
	lcd_command(LCD_MOVE_CURSOR_LEFT);
	s_display_chars--;
}

void display_clear(void) {
	lcd_clrscr();
	s_display_chars = 0u;
}

void display_test() {
	lcd_clrscr();
	s_display_chars = 0u;
	lcd_gotoxy(0, 0);
	display_write("Hello!");
	_delay_ms(1000);
}

void display_show_idle(void)
{
	display_show_idle_input("");
}

void display_show_idle_input(const char *input)
{
	if (input == (const char *)0)
	{
		input = "";
	}

	display_show_two_lines("Choose the floor", input);
}

void display_show_floor(uint8_t floor)
{
	char floor_line[3];

	floor_line[0] = (char)('0' + (floor / 10u));
	floor_line[1] = (char)('0' + (floor % 10u));
	floor_line[2] = '\0';

	display_show_two_lines("Current floor:", floor_line);
}

void display_show_door_open(void)
{
	display_show_two_lines("Door open", "");
}

void display_show_door_closing(void)
{
	display_show_two_lines("Door closing", "");
}

void display_show_obstacle(void)
{
	display_show_two_lines("Obstacle", "detected");
}

void display_show_same_floor_fault(void)
{
	display_show_two_lines("Same floor", "");
}

static void display_show_two_lines(const char *line_1, const char *line_2)
{
	display_clear();
	lcd_gotoxy(0, 0);
	display_write(line_1);

	if ((line_2 != (const char *)0) && (line_2[0] != '\0'))
	{
		lcd_gotoxy(0, 1);
		display_write(line_2);
	}
}