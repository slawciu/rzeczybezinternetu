#include <avr/io.h>

int main(void)
{
	// pb0 - output
	DDRB |= (1 << PB0);
	
	// pb3 - input
	DDRB &= (1 << PB3);
	
	// pull-up for pb3
	PORTB |= (1 << PB3);
	
    while (1)
    {
	    if (PINB & (1 << PB3))
	    {
		    PORTB |= (1 << PB0);
	    }
	    else
	    {
		    PORTB &= ~(1 << PB0);
	    }
    }
}

