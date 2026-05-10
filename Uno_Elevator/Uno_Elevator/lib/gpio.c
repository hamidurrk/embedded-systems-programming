/*
 * gpio.c
 *
 * Created: 4/12/2026 4:21:38 AM
 *  Author: Hamidur
 */

#include "gpio.h"

#include <avr/io.h>

typedef struct
{
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	volatile uint8_t *pin_reg;
	uint8_t bit;
} gpio_pin_t;

static uint8_t gpio_resolve_pin(uint8_t pin, gpio_pin_t *pin_cfg)
{
	if ((pin >= 2U) && (pin <= 7U))
	{
		pin_cfg->ddr = &DDRD;
		pin_cfg->port = &PORTD;
		pin_cfg->pin_reg = &PIND;
		pin_cfg->bit = pin;
		return 1U;
	}

	if ((pin >= 8U) && (pin <= 13U))
	{
		pin_cfg->ddr = &DDRB;
		pin_cfg->port = &PORTB;
		pin_cfg->pin_reg = &PINB;
		pin_cfg->bit = (uint8_t)(pin - 8U);
		return 1U;
	}

	return 0U;
}

void pin_mode(uint8_t pin, uint8_t mode)
{
	gpio_pin_t pin_cfg;
	uint8_t bit_mask;

	if (gpio_resolve_pin(pin, &pin_cfg) == 0U)
	{
		return;
	}

	bit_mask = (uint8_t)(1U << pin_cfg.bit);

	if (mode == OUTPUT)
	{
		*pin_cfg.ddr |= bit_mask;
	}
	else
	{
		*pin_cfg.ddr &= (uint8_t)~bit_mask;
		*pin_cfg.port &= (uint8_t)~bit_mask;
	}
}

void digital_write(uint8_t pin, uint8_t value)
{
	gpio_pin_t pin_cfg;
	uint8_t bit_mask;

	if (gpio_resolve_pin(pin, &pin_cfg) == 0U)
	{
		return;
	}

	bit_mask = (uint8_t)(1U << pin_cfg.bit);

	if (value == HIGH)
	{
		*pin_cfg.port |= bit_mask;
	}
	else
	{
		*pin_cfg.port &= (uint8_t)~bit_mask;
	}
}

uint8_t digital_read(uint8_t pin)
{
	gpio_pin_t pin_cfg;
	uint8_t bit_mask;

	if (gpio_resolve_pin(pin, &pin_cfg) == 0U)
	{
		return LOW;
	}

	bit_mask = (uint8_t)(1U << pin_cfg.bit);

	if ((*pin_cfg.pin_reg & bit_mask) != 0U)
	{
		return HIGH;
	}

	return LOW;
}