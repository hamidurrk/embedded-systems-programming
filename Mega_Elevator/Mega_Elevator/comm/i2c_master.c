/*
 * i2c_master.c
 *
 * Created: 4/11/2026 1:38:22 AM
 *  Author: Hamidur
 */ 

#include "i2c_master.h"

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#include "elevator_protocol.h"
#include "mcu.h"

#define I2C_MASTER_SCL_HZ (100000UL)
#define I2C_MASTER_SCL_HALF_PERIOD_US (5u)   /* 1/(2*100kHz) = 5us */
#define I2C_BUS_RECOVERY_CLOCKS       (9u)

static uint8_t i2c_master_start_write(uint8_t slave_address_7bit);
static uint8_t i2c_master_write_byte(uint8_t data);
static void i2c_master_stop(void);
static void i2c_master_bus_recover(void);
static void send_cmd(uint8_t cmd);

void i2c_master_init(void)
{
	/* Prescaler = 1 */
	TWSR = 0x00u;

	/* SCL = F_CPU / (16 + 2*TWBR*prescaler) */
	TWBR = (uint8_t)((F_CPU / I2C_MASTER_SCL_HZ - 16UL) / 2UL);

	/* Enable TWI peripheral */
	TWCR = (1u << TWEN);
}

void i2c_master_reinit(void)
{
	/* Disable TWI so we can bit-bang SCL for bus recovery */
	TWCR = 0x00u;

	i2c_master_bus_recover();
	i2c_master_init();
	_delay_ms(1.0);
}

void i2c_master_send_command(uint8_t command)
{
	send_cmd(command);
}

static uint8_t i2c_master_start_write(uint8_t slave_address_7bit)
{
	uint8_t status;

	TWCR = (1u << TWINT) | (1u << TWSTA) | (1u << TWEN);
	while ((TWCR & (1u << TWINT)) == 0u)
	{
	}

	status = (uint8_t)(TWSR & 0xF8u);
	if ((status != TW_START) && (status != TW_REP_START))
	{
		return 0u;
	}

	TWDR = (uint8_t)(slave_address_7bit << 1);
	TWCR = (1u << TWINT) | (1u << TWEN);
	while ((TWCR & (1u << TWINT)) == 0u)
	{
	}

	status = (uint8_t)(TWSR & 0xF8u);
	if (status != TW_MT_SLA_ACK)
	{
		return 0u;
	}

	return 1u;
}

static uint8_t i2c_master_write_byte(uint8_t data)
{
	TWDR = data;
	TWCR = (1u << TWINT) | (1u << TWEN);
	while ((TWCR & (1u << TWINT)) == 0u)
	{
	}

	return (((TWSR & 0xF8u) == TW_MT_DATA_ACK) ? 1u : 0u);
}

static void i2c_master_bus_recover(void)
{
	uint8_t i;

	/* Drive SCL as output, SDA as input with pull-up (PD0=SCL, PD1=SDA on Mega) */
	DDRD  |= (1u << PD0);
	DDRD  &= (uint8_t)(~(1u << PD1));
	PORTD |= (1u << PD1);

	/* Clock out up to 9 pulses to release a stuck slave */
	for (i = 0u; i < I2C_BUS_RECOVERY_CLOCKS; i++)
	{
		PORTD &= (uint8_t)(~(1u << PD0));
		_delay_us(I2C_MASTER_SCL_HALF_PERIOD_US);
		PORTD |= (1u << PD0);
		_delay_us(I2C_MASTER_SCL_HALF_PERIOD_US);

		/* SDA released means slave freed the bus */
		if ((PIND & (1u << PD1)) != 0u)
		{
			break;
		}
	}

	/* Issue a STOP: SDA low -> SCL high -> SDA high */
	DDRD  |= (1u << PD1);
	PORTD &= (uint8_t)(~(1u << PD1));
	_delay_us(I2C_MASTER_SCL_HALF_PERIOD_US);
	PORTD |= (1u << PD0);
	_delay_us(I2C_MASTER_SCL_HALF_PERIOD_US);
	PORTD |= (1u << PD1);
	_delay_us(I2C_MASTER_SCL_HALF_PERIOD_US);

	/* Return pins to input — TWI peripheral takes them over on i2c_master_init() */
	DDRD  &= (uint8_t)(~((1u << PD0) | (1u << PD1)));
	PORTD &= (uint8_t)(~((1u << PD0) | (1u << PD1)));
}

static void i2c_master_stop(void)
{
	TWCR = (1u << TWINT) | (1u << TWEN) | (1u << TWSTO);
	while ((TWCR & (1u << TWSTO)) != 0u)
	{
	}
}

static void send_cmd(uint8_t cmd)
{
	if (i2c_master_start_write(UNO_I2C_ADDRESS) == 0u)
	{
		i2c_master_stop();
		return;
	}

	if (i2c_master_write_byte(cmd) == 0u)
	{
		i2c_master_stop();
		return;
	}

	i2c_master_stop();
}
