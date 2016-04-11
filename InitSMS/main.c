#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#define F_CPU 8000000UL
#include <util/delay.h>

#define UBRR_VALUE_GSM 3 //(((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void USART0Init(void)
{
    // Set baud rate
    UBRR0L = (uint8_t)UBRR_VALUE_GSM;
    UBRR0H = (uint8_t)(UBRR_VALUE_GSM>>8);
    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C=(1<<URSEL0)|(1<<2)|(1<<1);
    //enable reception and RC complete interrupt
    UCSR0B |= (1<<RXEN0)|(1<<RXCIE0)|(1<<TXEN0);
}

void TurnGSMOn()
{
    PORTC |= (1<<PC0);
    _delay_ms(200);
    PORTC &= ~(1<<PC0);
    _delay_ms(1000);
    PORTC |= (1<<PC0);
}

int main(void)
{
    USART0Init();
    TurnGSMOn();
    
    while (1) 
    {
    }
}
