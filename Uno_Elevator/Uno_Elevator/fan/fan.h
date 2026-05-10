/*
 * fan.h
 *
 * Created: 4/12/2026 3:32:52 AM
 *  Author: Hamidur
 */ 


#ifndef FAN_H_
#define FAN_H_

#include <stdint.h>

void fan_init(void);
void fan_on(void);
void fan_off(void);
void fan_on_for_ms(uint16_t duration_ms);




#endif /* FAN_H_ */