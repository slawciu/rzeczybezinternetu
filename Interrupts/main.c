#include <avr/io.h>
#include <avr/interrupt.h>

int main(void)
{
	// set pd2 (INT0) as input
	DDRD &= (1 << PD2);

	// pull-up for pd2
	PORTD |= (1 << PD2);

	// set pb0 and pb1 pins as output
	DDRB |= (1 << PB0) | (1 << PB1);

	// turn off led connected to pb0
	PORTB |= (1 << PB0);

	// turn on led connected to pb1
	PORTB &= ~(1 << PB1);
	
	// enable INT0 interrupt
	GICR |= (1 << INT0);
	
	// enable global interrupts
	sei();

	while (1)
	{
	}
}

ISR(INT0_vect)
{
	PORTB ^= (1 << PB0);
	PORTB ^= (1 << PB1);
}

