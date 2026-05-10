/*
 * i2c_slave.h
 *
 * Created: 4/11/2026 1:46:39 AM
 *  Author: Hamidur
 */ 


#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_

#include <stdint.h>

void i2c_slave_init(void);
void i2c_slave_task(void);
void i2c_slave_handle_command(uint8_t command);

#endif /* I2C_SLAVE_H_ */