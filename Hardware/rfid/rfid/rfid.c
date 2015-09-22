
/********************************************************************************/
# define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include<avr/interrupt.h>


/********************************************************************************/
/*   INITIALIZATION
FUNCTION                
*/
/********************************************************************************/


void init (void) // set all registers
{
  DDRB   = 0x03;   // port b(0,1) outputs [Led 1&2], b(2..7) inputs [PINB2 = SWITCH]
  PORTB  = 0x00;
  
  DDRD   = 0x00;   // port d inputs
  PORTD  = 0x00;   //          


  GIMSK  = 0x20;   // Pin chande interrupt enabled
  PCMSK  = 0x04;   // pin change interrupt mask, selecting pin b2
 
   
  MCUCR  = 0x00;   // no sleep or standby modes 
  SREG   = 0x80;   // global interrupt enable
}


void USART_Init( unsigned int baud )
{
/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<USBS)|(3<<UCSZ0);
}

void put_char(char byte)
{
	while ( !( UCSRA & (1<<UDRE)) );	// Wait for empty transmit buffer 

	UDR = byte;							// Put data into buffer, sends the data 
}

char get_char(void)
{
	while(!(UCSRA & (1<<RXC)));			// Wait for recieve buffer 
	
	return UDR;							// Return data from Data Register 
}

/********************************************************************************/
/*       */
/*   MAIN    */
/*       */
/********************************************************************************/

 

int main (void)
{

 char data;
 init();
 USART_Init(12); // 38400 bps at F_Osc 8MHz

 sei();



 PORTB |= (1<<1);	// led 2 off 

 while(1)
 {
	// blink leds when receiving data from tags
	data = get_char();
	PORTB &= ~(1<<0); 	// led 1 on
	data = get_char();
	PORTB |= (1<<0);	//led 1 off
 }
}

 
/********************************************************************************/
/*    ISR    */
/********************************************************************************/


ISR (PCINT_vect)//pin change on pin b2
{
	cli(); 

   if (!(PINB & 4))
   {
		PORTB &= ~(1<<1); //led 1 on
		 // <02>0008002101010000<F35E> loop on
		put_char(0x02);
 		put_char(0x00);
 		put_char(0x08);
 		put_char(0x00);
 		put_char(0x21);
 		put_char(0x01);
 		put_char(0x01);
 		put_char(0x00);
 		put_char(0x00);
 		put_char(0xF3);
 		put_char(0x5E);
   }
   
   if (PINB & 4)
   {
		 PORTB |= (1<<1);//led 1 off
		 // <02>0008002001010000<F81A> select tag loop off
		put_char(0x02);
 		put_char(0x00);
 		put_char(0x08);
 		put_char(0x00);
 		put_char(0x20);
 		put_char(0x01);
 		put_char(0x01);
 		put_char(0x00);
 		put_char(0x00);
 		put_char(0xF8);
 		put_char(0x1A);
   }

	sei();
}
