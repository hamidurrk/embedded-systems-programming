/*
 * elevator_commands.h
 *
 * Created: 4/11/2026 1:38:56 AM
 *  Author: Hamidur
 */ 


#ifndef ELEVATOR_COMMANDS_H_
#define ELEVATOR_COMMANDS_H_

#define CMD_IDLE          0x00
#define CMD_MOVING_UP     0x01
#define CMD_MOVING_DOWN   0x02
#define CMD_DOOR_OPEN     0x03
#define CMD_DOOR_CLOSE    0x04
#define CMD_OBSTACLE      0x05
#define CMD_PLAY_MELODY   0x06
#define CMD_STOP_MELODY   0x07
#define CMD_FAULT         0x08
#define CMD_SLEEP         0x09
#define CMD_WAKE          0x0A

/* Test-mode commands */
#define CMD_TEST_LED       0x20
#define CMD_TEST_BUZZER    0x21
#define CMD_TEST_FAN       0x22
#define CMD_TEST_SONAR     0x23
#define CMD_TEST_DOTMATRIX 0x24

#endif /* ELEVATOR_COMMANDS_H_ */