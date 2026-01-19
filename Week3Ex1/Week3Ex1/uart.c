/*
 * CFile1.c
 *
 * Created: 12.1.2026 11:43:01
 *  Author: x144622
 */ 
# include "mcu.h"
# include "uart.h"

# include <avr/io.h>
# include <stdio.h># define MYUBRR (F_CPU/16/UART_BAUD_RATE - 1)static void USART_init( uint16_t ubrr ) // USART initiation p .206
{
 UBRR0H = ( unsigned char )( ubrr >> 8) ; 
 UBRR0L = ( unsigned char )ubrr ; 

 /* Enable receiver and transmitter on RX0 and TX0 */
 UCSR0B |= (1 << RXEN0 ) | (1 << TXEN0) ; // RX complete interrupt enable //

 /* Set frame format : 8 bit data , 2 stop bit */
UCSR0C |= (1 << USBS0 ) | (3 << UCSZ00 ) ;
 }
 
 static int USART_Transmit ( char data , FILE * stream ) 
  {
 /* Wait until the transmit buffer is empty */
 while (!( UCSR0A & (1 << UDRE0))) 
 {
 ;
 }

 UDR0 = data ;

	return 0;
}

static int USART_Receive( FILE * stream )
 {
// Wait until data is received
while (!( UCSR0A & (1 << RXC0 ) ) ) 
;

char c = UDR0;

if (c == '\r')
c = '\n';


// Return received byte
 return c;
 }FILE uart_output = FDEV_SETUP_STREAM ( USART_Transmit , NULL , _FDEV_SETUP_WRITE ) ;
FILE uart_input = FDEV_SETUP_STREAM ( NULL , USART_Receive , _FDEV_SETUP_READ );uint8_t setup_uart_io( void )
 {
	USART_init ( MYUBRR ) ;
	
	stdout = & uart_output ;
	stdin = & uart_input ;
	
	 return 0;
 }