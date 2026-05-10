/*
 * sonar.c
 *
 * Created: 4/12/2026 3:33:14 AM
 *  Author: Hamidur
 */ 


#include "sonar.h"

#include <avr/io.h>

#include "../config/pins.h"
#include "delay.h"

#define SONAR_ECHO_TIMEOUT_US  30000U
#define SONAR_ECHO_TIMEOUT_TICKS  ((uint16_t)(SONAR_ECHO_TIMEOUT_US * 2U))
#define SONAR_TICKS_PER_CM       116U

#define INPUT   0U
#define OUTPUT  1U
#define LOW     0U
#define HIGH    1U

static uint8_t sonar_resolve_pin(
	uint8_t pin,
	volatile uint8_t **ddr,
	volatile uint8_t **port,
	volatile uint8_t **pin_reg,
	uint8_t *bitmask);
static void pin_mode(uint8_t pin, uint8_t mode);
static void digital_write(uint8_t pin, uint8_t value);
static uint8_t digital_read(uint8_t pin);

static void sonar_timer1_start(void)
{
	/* Timer1: normal mode, prescaler=8 -> tick = 0.5us at 16MHz. */
	TCCR1A = 0U;
	TCCR1B = 0U;
	TCNT1 = 0U;
	TCCR1B = (1U << CS11);
}

static void sonar_timer1_stop(void)
{
	TCCR1B = 0U;
}

static uint8_t sonar_wait_for_echo_level(uint8_t expected_level, uint16_t timeout_us)
{
	while (timeout_us > 0U)
	{
		if (digital_read(SONAR_ECHO) == expected_level)
		{
			return 1U;
		}

		DELAY_us(1);
		timeout_us--;
	}

	return 0U;
}

static uint8_t sonar_resolve_pin(
	uint8_t pin,
	volatile uint8_t **ddr,
	volatile uint8_t **port,
	volatile uint8_t **pin_reg,
	uint8_t *bitmask)
{
	if ((ddr == (volatile uint8_t **)0) || (port == (volatile uint8_t **)0) ||
		(pin_reg == (volatile uint8_t **)0) || (bitmask == (uint8_t *)0))
	{
		return 0U;
	}

	/* Arduino MEGA mapping used by this project:
	 * D33 -> PC4, D35 -> PC2
	 */
	switch (pin)
	{
	case SONAR_TRIG:
		*ddr = &DDRC;
		*port = &PORTC;
		*pin_reg = &PINC;
		*bitmask = (1U << 4);
		return 1U;

	case SONAR_ECHO:
		*ddr = &DDRC;
		*port = &PORTC;
		*pin_reg = &PINC;
		*bitmask = (1U << 2);
		return 1U;

	default:
		return 0U;
	}
}

static void pin_mode(uint8_t pin, uint8_t mode)
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin_reg;
	uint8_t bitmask;

	if (sonar_resolve_pin(pin, &ddr, &port, &pin_reg, &bitmask) == 0U)
	{
		return;
	}

	if (mode == OUTPUT)
	{
		*ddr = (uint8_t)(*ddr | bitmask);
	}
	else
	{
		*ddr = (uint8_t)(*ddr & (uint8_t)(~bitmask));
		*port = (uint8_t)(*port & (uint8_t)(~bitmask));
	}
}

static void digital_write(uint8_t pin, uint8_t value)
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin_reg;
	uint8_t bitmask;

	if (sonar_resolve_pin(pin, &ddr, &port, &pin_reg, &bitmask) == 0U)
	{
		return;
	}

	if (value == HIGH)
	{
		*port = (uint8_t)(*port | bitmask);
	}
	else
	{
		*port = (uint8_t)(*port & (uint8_t)(~bitmask));
	}
}

static uint8_t digital_read(uint8_t pin)
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin_reg;
	uint8_t bitmask;

	if (sonar_resolve_pin(pin, &ddr, &port, &pin_reg, &bitmask) == 0U)
	{
		return LOW;
	}

	return (((*pin_reg & bitmask) != 0U) ? HIGH : LOW);
}

void sonar_init(void)
{
	pin_mode(SONAR_TRIG, OUTPUT);
	digital_write(SONAR_TRIG, LOW);

	pin_mode(SONAR_ECHO, INPUT);
}

uint16_t sonar_measure_distance_cm(void)
{
	uint16_t echo_ticks;

	sonar_init();

	if (sonar_wait_for_echo_level(LOW, SONAR_ECHO_TIMEOUT_US) == 0U)
	{
		return SONAR_INVALID_DISTANCE_CM;
	}

	/* Generate a 10us trigger pulse. */
	digital_write(SONAR_TRIG, LOW);
	DELAY_us(2);
	digital_write(SONAR_TRIG, HIGH);
	DELAY_us(10);
	digital_write(SONAR_TRIG, LOW);

	if (sonar_wait_for_echo_level(HIGH, SONAR_ECHO_TIMEOUT_US) == 0U)
	{
		return SONAR_INVALID_DISTANCE_CM;
	}

	sonar_timer1_start();

	while (digital_read(SONAR_ECHO) == HIGH)
	{
		if (TCNT1 >= SONAR_ECHO_TIMEOUT_TICKS)
		{
			sonar_timer1_stop();
			return SONAR_INVALID_DISTANCE_CM;
		}
	}

	echo_ticks = TCNT1;
	sonar_timer1_stop();

	if (echo_ticks == 0U)
	{
		return SONAR_INVALID_DISTANCE_CM;
	}
	
	/* 116 timer ticks ~= 1 cm (round trip), with tick = 0.5us. */
	return (uint16_t)((echo_ticks + (SONAR_TICKS_PER_CM / 2U)) / SONAR_TICKS_PER_CM);
}
