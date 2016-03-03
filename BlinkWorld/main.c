#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>

/*
	Hardware setup:
	Cristal: 8 MHz
	Power supply: 5V DC
	
	LEDs:
	[PB0]---|<|---[100R]---VCC
	[PB1]---|<|---[100R]---VCC
*/

int main(void)
{
	// set pb0 and pb1 pins as output
	DDRB |= (1 << PB0) | (1 << PB1);

	// turn off led connected to pb0
	PORTB |= (1 << PB0);
	
	// turn on led connected to pb1
	PORTB &= ~(1 << PB1);

	while (1)
	{
		// switch pb0 led state
		PORTB ^= (1 << PB0);
		
		// switch pb1 led state
		PORTB ^= (1 << PB1);

		_delay_ms(500);
	}
}

