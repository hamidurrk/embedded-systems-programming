/*
 * app.c
 *
 * Created: 4/11/2026 1:45:38 AM
 *  Author: Hamidur
 */ 

#include "app.h"
#include "../comm/i2c_slave.h"
#include "../led/led.h"
#include "../buzzer/buzzer.h"
#include "../fan/fan.h"
#include "../dotmatrix/dotmatrix.h"
#include "../elevator/elevator.h"

void app_init(void)
{
	setup_uart_io();
	led_init();
	buzzer_init();
	fan_init();
	dotmatrix_init();
	elevator_init();
	i2c_slave_init();
}

void app_task(void)
{
	i2c_slave_task();
}