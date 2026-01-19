/*
 * main_task1.c
 *
 * Created: 19.1.2026 11:35:56
 *  Author: x144622
 */ 

# include "mcu.h"
# include "uart.h"

# include <avr/io.h>
# include <util/delay.h> /* for delay */

# include "bit_ops.h"
# include "board_config.h"
# include "task1_pin_config.h"

static void handle_error ( uint8_t return_code )
 {
 // Non - zero return code indicates critical fault
 if ( return_code )
 {
 while (1) ;
 }
 }

int main(void) {
	uint8_t rc = setup_uart_io();
	handle_error ( rc ) ;
	
	printf (" Configuring IO .\r\n") ;
	
	CLEAR_BIT (LED_8_PORT, LED_8_PIN) ;
	SET_BIT (LED_8_DIRECTION, LED_8_PIN) ;
	
	CLEAR_BIT (LED_9_PORT, LED_9_PIN) ;
	SET_BIT (LED_9_DIRECTION, LED_9_PIN) ;
	
	CLEAR_BIT (LED_10_PORT, LED_10_PIN) ;
	SET_BIT (LED_10_DIRECTION, LED_10_PIN) ;
	
	CLEAR_BIT (LED_11_PORT, LED_11_PIN) ;
	SET_BIT (LED_11_DIRECTION, LED_11_PIN) ;
	
	printf (" Configuration done .\r\n") ;
	
	while(1) {
		SET_BIT(LED_8_PORT, LED_8_PIN);
		_delay_ms(1000);
		CLEAR_BIT (LED_8_PORT, LED_8_PIN) ;
		_delay_ms(1000);
		printf (" LED 8.\r\n") ;
		
		SET_BIT(LED_9_PORT, LED_9_PIN);
		_delay_ms(1000);
		CLEAR_BIT (LED_9_PORT, LED_9_PIN) ;
		_delay_ms(1000);
		printf (" LED 9.\r\n") ;
		
		SET_BIT(LED_10_PORT, LED_10_PIN);
		_delay_ms(1000);
		CLEAR_BIT (LED_10_PORT, LED_10_PIN) ;
		_delay_ms(1000);
		printf (" LED 10.\r\n") ;
		
		SET_BIT(LED_11_PORT, LED_11_PIN);
		_delay_ms(1000);
		CLEAR_BIT (LED_11_PORT, LED_11_PIN) ;
		_delay_ms(1000);
		printf (" LED 11.\r\n") ;
		
	}
}