/*
 * elevator.h
 *
 * Created: 4/11/2026 1:36:00 AM
 *  Author: Hamidur
 */ 


#ifndef ELEVATOR_H_
#define ELEVATOR_H_

#include <stdint.h>

typedef enum
{
	STATE_IDLE,
	STATE_GOING_UP,
	STATE_GOING_DOWN,
	STATE_DOOR_OPENING,
	STATE_OBSTACLE_DETECTION,
	STATE_DOOR_CLOSING,
	STATE_FAULT
} ElevatorState;

void elevator_init(void);
void elevator_task(void);

#endif /* ELEVATOR_H_ */