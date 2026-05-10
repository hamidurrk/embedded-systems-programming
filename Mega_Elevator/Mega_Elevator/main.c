/*
 * Mega_Elevator.c
 *
 * Created: 4/11/2026 1:30:45 AM
 * Author : Hamidur
 */ 

#include <avr/io.h>
#include "app/app.h"

int main(void)
{
	app_init();
    while (1) 
    {
		app_task();
    }
}

