/*
 * gpio.h
 *
 * Created: 4/12/2026 4:21:38 AM
 *  Author: Hamidur
 */


#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

#define INPUT   0U
#define OUTPUT  1U

#define LOW     0U
#define HIGH    1U

void pin_mode(uint8_t pin, uint8_t mode);
void digital_write(uint8_t pin, uint8_t value);
uint8_t digital_read(uint8_t pin);

#endif /* GPIO_H_ */