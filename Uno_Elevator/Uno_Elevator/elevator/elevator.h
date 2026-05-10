/*
 * elevator.h
 *
 * Created: 4/12/2026 9:14:57 PM
 *  Author: Hamidur
 */ 


#ifndef ELEVATOR_H_
#define ELEVATOR_H_




#include <stdint.h>

void elevator_init(void);
void elevator_set_command(uint8_t command);
void elevator_set_melody_enabled(uint8_t enabled);
void elevator_enter_sleep(void);
void elevator_exit_sleep(void);
void elevator_task(void);
uint8_t elevator_get_current_command(void);

#endif /* ELEVATOR_H_ */