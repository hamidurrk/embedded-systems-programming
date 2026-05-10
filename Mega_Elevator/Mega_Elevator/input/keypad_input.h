/*
 * keypad_input.h
 *
 * Created: 4/11/2026 1:36:33 AM
 *  Author: Hamidur
 */ 


#ifndef KEYPAD_INPUT_H_
#define KEYPAD_INPUT_H_

#include <stdint.h>

#define KEYPAD_INPUT_READ_NONE       (0u)
#define KEYPAD_INPUT_READ_CONFIRMED  (1u)
#define KEYPAD_INPUT_READ_PENDING    (2u)

void keypad_input_init(void);
void keypad_input_reset_accumulator(void);
const char *keypad_input_get_key(void);
uint8_t keypad_input_get_key_if_pressed(uint8_t *key_out);
uint8_t keypad_input_get_key_nonblocking(uint8_t *key_out);

uint8_t keypad_input_read_floor(uint8_t *floor_out);
uint8_t keypad_input_read_floor_nonblocking(uint8_t *floor_out);
const char *keypad_input_floor_to_display(uint8_t read_status, uint8_t floor_out);

#endif /* KEYPAD_INPUT_H_ */