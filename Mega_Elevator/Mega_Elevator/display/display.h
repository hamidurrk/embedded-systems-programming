/*
 * display.h
 *
 * Created: 4/11/2026 1:37:36 AM
 *  Author: Hamidur
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

void display_init(void);
void display_write(const char* string);
void display_delete_last(void);
void display_clear(void);
void display_test(void);
void display_show_idle(void);
void display_show_idle_input(const char *input);
void display_show_floor(uint8_t floor);
void display_show_door_open(void);
void display_show_door_closing(void);
void display_show_obstacle(void);
void display_show_same_floor_fault(void);

#endif /* DISPLAY_H_ */