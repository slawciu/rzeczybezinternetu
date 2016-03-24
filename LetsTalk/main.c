#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#define USART_BAUDRATE 19200// 115200
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) // 25

void USART0Init(void)
{
    // Set baud rate
    UBRR0L = (uint8_t)UBRR_VALUE;
    UBRR0H = (uint8_t)(UBRR_VALUE>>8);
    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C=(1<<URSEL0)|(1<<2)|(1<<1);
    //enable reception and RC complete interrupt
    UCSR0B |= (1<<RXEN0)|(1<<RXCIE0)|(1<<TXEN0);
}

void UsartFlush()
{
    UCSR0B &= ~(1<<RXEN0);
    UCSR0B |= (1<<RXEN0);
}

void UsartWrite(char* text)
{
    while(*text)
    {
        while(!(UCSR0A & (1<<UDRE0)))
        {
            //Do nothing
        }

        //Now write the data to USART buffer
        UDR0=*text++;
    }
}

int main(void)
{
    USART0Init();
    
    // set pb0 and pb1 pins as output
    DDRB |= (1 << PB0) | (1 << PB1);

    // turn off leds connected to pb
    PORTB |= (1 << PB0) | (1 << PB1);

    // initialize interrupts
    sei();
       
    while (1) 
    {
    }
}

// onUsart
ISR(USART0_RXC_vect)
{
    char temp;
    
    temp = UDR0;
    
    if (temp == '0')
    {
        // switch led state connected to pb0
        PORTB ^= (1 << PB0);
    }
    else if (temp == '1')
    {
        // switch led state connected to pb1
        PORTB ^= (1 << PB1);
    }
}

