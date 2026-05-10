/*
 * max7219.h
 *
 * Created: 4/12/2026 5:44:34 AM
 *  Author: Hamidur
 */ 


#ifndef MAX7219_H_
#define MAX7219_H_


#include <stdint.h>
#include <stdbool.h>

void max7219_init(void);
void max7219_clear(void);
void max7219_set_row(uint8_t row, uint8_t value);

void max7219_clear_buffer(bool buffer[8][8]);
void max7219_show_rotated_clockwise(bool buffer[8][8]);
void max7219_draw_char(bool buffer[8][8], int xOffset, char c);



#endif /* MAX7219_H_ */