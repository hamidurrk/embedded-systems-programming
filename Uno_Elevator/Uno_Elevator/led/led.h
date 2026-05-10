/*
 * led.h
 *
 * Created: 4/11/2026 1:48:01 AM
 *  Author: Hamidur
 */ 


#ifndef LED_H_
#define LED_H_

#include <stdint.h>

void led_init(void);
void led_on(uint8_t pin);
void led_off(uint8_t pin);
void led_on_for_ms(uint16_t duration_ms);
void led_all_on(void);
void led_all_off(void);

void led_set_idle(void);
void led_set_moving_up(void);
void led_set_moving_down(void);
void led_set_door_open(void);
void led_set_door_close(void);
void led_blink_obstacle_three_times(void);

#endif /* LED_H_ */