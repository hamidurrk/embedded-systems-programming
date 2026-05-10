/*
 * buzzer.h
 *
 * Created: 4/11/2026 1:48:23 AM
 *  Author: Hamidur
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

#include <stdint.h>

#define BUZZER_MELODY_MOVING    0U
#define BUZZER_MELODY_DOOR      1U
#define BUZZER_MELODY_OBSTACLE  2U

void buzzer_init(void);
void buzzer_tone(uint16_t hz);
void buzzer_start_melody(void);
void buzzer_stop(void);
void buzzer_select_melody(uint8_t melody_id);
void buzzer_set_melody_loop(uint8_t enabled);
void buzzer_task(void);

#endif /* BUZZER_H_ */