/*
 * i2c_master.h
 *
 * Created: 4/11/2026 1:38:01 AM
 *  Author: Hamidur
 */ 


#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#include <stdint.h>

void i2c_master_init(void);
void i2c_master_reinit(void);
void i2c_master_send_command(uint8_t command);

#endif /* I2C_MASTER_H_ */