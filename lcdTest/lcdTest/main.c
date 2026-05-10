/*
 * Exercise 3_LCD_Keypad.c
 *
 * Created: 1/12/2025 11:47:41 AM
 * Author : Anafi Nur'aini, Aleksei Romanenko
 */ 

#include "mcu.h"
#include "uart.h"

#include "keypad.h"
#include "lcd.h" // lcd header file made by Peter Fleury
#include "delay.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define NO_KEY_PRESSED  (0xFF)

/// There is not much we can do for now. This function will be improved in future.
static void handle_error(uint8_t return_code)
{
	// Non-zero return code indicates critical fault
	if (return_code)
	{
		while(1);
	}
}

#define LCD_MAX_STRING (32)

/**
 * Protected write to LCD that checks that provided pointer is a valid null-terminated string.
 * @param string Pointer to the string that should be printed
 */
static void write_to_lcd(const char *string) {
	uint8_t len = strnlen(string, LCD_MAX_STRING);
	if (LCD_MAX_STRING == len) {
		printf("Failed to print LCD string. Too big or lacks NULL-terminator.\r\n");
		// Since we have null-terminator we print the bad string one character at a time.
		for (uint8_t i = 0; i < len; i++)
		{
		    printf("%c", string[i]);
		}
		printf("\r\n");
		handle_error(1);
	} else {
	    printf("LCD output: '%s'\r\n", string);
	    lcd_puts(string);
	}
}

// Initialize LCD and Keypad
void setup() {
	// Initialize serial port for standard library
	uint8_t rc = setup_uart_io();
	handle_error(rc);

	// Initialize LCD
	printf("Initializing LCD driver\r\n");
	lcd_init(LCD_DISP_ON);
	printf("Initializing LCD driver done.\r\n");
	lcd_clrscr();
	write_to_lcd("Ready");
	printf("LCD Ready.\r\n");
	_delay_ms(1000); // Delay to display initial message

	// Initialize Keypad
	KEYPAD_Init();
}

int main(void) {
	static char key_str[32];
	setup();

	uint32_t memory = 0;

	while (1) {
		// Read raw signal from keypad
		uint8_t key_signal = KEYPAD_GetKey();
		if (key_signal != NO_KEY_PRESSED) { // Assuming 0xFF means no key pressed
			printf("Keypad: %c\r\n", key_signal);

			// Check if it's a numeric key (0 to 9) then store in memory variable
			if (key_signal >= '0' && key_signal <= '9') {
				// Convert key to numeric value
				uint8_t key_value = key_signal - '0'; // Convert ASCII value to numeric value
				// Prevent integer overflows
				if (memory < (C_UINT32_MAX - key_value)/10) {
					memory *= 10;
					memory += key_value;
				}
			}

			// Use * key to clear memory
			if (key_signal == '*') {
				memory = 0;
			}
		
			lcd_clrscr();
			// Safely format string before writing to LCD
			snprintf(key_str, sizeof(key_str), "%c", key_signal); // Convert numeric value to string

			// Display the key on the LCD
			write_to_lcd(key_str);
			
			//write_to_lcd("I <3 Bushra");
			snprintf(key_str, sizeof(key_str), "Mem: %"PRIu32"", memory); // Convert numeric value to string

			// Display memory on LCD
			lcd_gotoxy(0, 1);
			write_to_lcd(key_str);
		}
	}


	return 0;
}
