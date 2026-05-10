/*
 * sonar.h
 *
 * Created: 4/12/2026 3:33:03 AM
 *  Author: Hamidur
 */ 


#ifndef SONAR_H_
#define SONAR_H_


#include <stdint.h>

/* Returned when no valid echo pulse is captured within timeout. */
#define SONAR_INVALID_DISTANCE_CM  0xFFFFU

void sonar_init(void);
uint16_t sonar_measure_distance_cm(void);




#endif /* SONAR_H_ */