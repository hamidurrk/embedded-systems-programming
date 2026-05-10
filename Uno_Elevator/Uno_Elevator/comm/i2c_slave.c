/*
 * i2c_slave.c
 *
 * Created: 4/11/2026 1:46:09 AM
 *  Author: Hamidur
 */ 

#include "i2c_slave.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/atomic.h>
#include <util/twi.h>

#include "elevator_commands.h"

#include "../elevator/elevator.h"
#include "../test/test.h"

#ifndef I2C_SLAVE_ADDRESS
#define I2C_SLAVE_ADDRESS  0x08U
#endif

static volatile uint8_t g_pending_command = CMD_IDLE;
static volatile uint8_t g_command_pending = 0U;
static uint8_t g_active_command = CMD_IDLE;

static void i2c_slave_rearm(void)
{
	TWCR = (1U << TWINT) | (1U << TWEA) | (1U << TWEN) | (1U << TWIE);
}

ISR(TWI_vect)
{
	uint8_t status = (uint8_t)(TWSR & 0xF8U);

	switch (status)
	{
		case TW_SR_SLA_ACK:
		case TW_SR_GCALL_ACK:
		case TW_SR_ARB_LOST_SLA_ACK:
		case TW_SR_ARB_LOST_GCALL_ACK:
			break;

		case TW_SR_DATA_ACK:
		case TW_SR_GCALL_DATA_ACK:
			g_pending_command = TWDR;
			g_command_pending = 1U;
			printf("[ISR] RX: 0x%02X\r\n", TWDR);
			break;

		case TW_SR_STOP:
		case TW_SR_DATA_NACK:
		case TW_SR_GCALL_DATA_NACK:
		default:
			break;
	}

	i2c_slave_rearm();
}

void i2c_slave_init(void)
{
	TWAR = (uint8_t)(I2C_SLAVE_ADDRESS << 1U);
	TWAMR = 0x00U;
	TWSR = 0x00U;

	i2c_slave_rearm();
	sei();
}

void i2c_slave_task(void)
{
	uint8_t command = CMD_IDLE;
	uint8_t has_pending = 0U;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (g_command_pending != 0U)
		{
			command = g_pending_command;
			g_command_pending = 0U;
			has_pending = 1U;
		}
	}

	if (has_pending != 0U)
	{
		i2c_slave_handle_command(command);
	}

	switch (g_active_command)
	{
		case CMD_TEST_LED:
			led_test();
			break;

		case CMD_TEST_BUZZER:
			buzzer_test();
			break;

		case CMD_TEST_FAN:
			fan_test();
			break;

		case CMD_TEST_DOTMATRIX:
			dotmatrix_test();
			break;

		default:
			elevator_task();
			break;
	}
}

void i2c_slave_handle_command(uint8_t command)
{
	switch (command)
	{
		case CMD_IDLE:
		case CMD_MOVING_UP:
		case CMD_MOVING_DOWN:
		case CMD_DOOR_OPEN:
		case CMD_DOOR_CLOSE:
		case CMD_OBSTACLE:
		case CMD_FAULT:
			elevator_set_command(command);
			g_active_command = CMD_IDLE;
			if (command != CMD_IDLE)
			{
				g_active_command = command;
			}
			printf("[I2C] Runtime command: 0x%02X\r\n", command);
			break;

		case CMD_SLEEP:
			printf("[I2C] Sleep command received\r\n");
			elevator_enter_sleep();
			g_active_command = CMD_IDLE;
			break;

		case CMD_WAKE:
			elevator_exit_sleep();
			g_active_command = CMD_IDLE;
			printf("[I2C] Wake command received\r\n");
			break;

		case CMD_PLAY_MELODY:
			elevator_set_melody_enabled(1U);
			printf("[I2C] Melody command: ON\r\n");
			break;

		case CMD_STOP_MELODY:
			elevator_set_melody_enabled(0U);
			printf("[I2C] Melody command: OFF\r\n");
			break;

		case CMD_TEST_LED:
			elevator_set_melody_enabled(0U);
			elevator_set_command(CMD_IDLE);
			led_test_reset();
			g_active_command = CMD_TEST_LED;
			printf("[I2C] Test command: LED\r\n");
			break;

		case CMD_TEST_BUZZER:
			elevator_set_melody_enabled(0U);
			elevator_set_command(CMD_IDLE);
			buzzer_test_reset();
			g_active_command = CMD_TEST_BUZZER;
			printf("[I2C] Test command: BUZZER\r\n");
			break;

		case CMD_TEST_FAN:
			elevator_set_melody_enabled(0U);
			elevator_set_command(CMD_IDLE);
			fan_test_reset();
			g_active_command = CMD_TEST_FAN;
			printf("[I2C] Test command: FAN\r\n");
			break;

		case CMD_TEST_DOTMATRIX:
			elevator_set_melody_enabled(0U);
			elevator_set_command(CMD_IDLE);
			dotmatrix_test_reset();
			g_active_command = CMD_TEST_DOTMATRIX;
			printf("[I2C] Test command: DOTMATRIX\r\n");
			break;

		default:
			elevator_set_melody_enabled(0U);
			elevator_set_command(CMD_IDLE);
			g_active_command = CMD_IDLE;
			printf("[I2C] Unknown command 0x%02X -> IDLE\r\n", command);
			break;
	}
}
