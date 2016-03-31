#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#define USART_BAUDRATE 19200// 115200
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) // 25

typedef enum 
{ 
    Idle = 0, 
    Led0On, 
    Led0Off, 
    Led1On, 
    Led1Off 
} States;

typedef enum 
{ 
    TurnLed0On = 'A', 
    TurnLed0Off = 'a', 
    TurnLed1On = 'B', 
    TurnLed1Off = 'b' 
} Messages;

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

volatile States state;

int main(void)
{
    state = Idle;
    
    USART0Init();
    
    // set pb0 and pb1 pins as output
    DDRB |= (1 << PB0) | (1 << PB1);

    // turn off leds connected to pb
    PORTB |= (1 << PB0) | (1 << PB1);

    // initialize interrupts
    sei();
    
    while (1)
    {
        switch (state)
        {
            case Idle:               
                break;
            case Led0Off:
                PORTB |= (1 << PB0);
                state = Idle;
                break;
            case Led0On:
                PORTB &= ~(1 << PB0);
                state = Idle;
                break;
            case Led1Off:
                PORTB |= (1 << PB0);
                state = Idle;
                break;
            case Led1On:
                PORTB &= ~(1 << PB0);
                state = Idle;
                break;
            default:
                break;
            
        }    
    }
}

// onUsart
ISR(USART0_RXC_vect)
{
    char messsage;
    
    messsage = UDR0;
    
    switch (messsage)
    {
        case TurnLed0Off:
            state = Led0Off;
            break;
        case TurnLed0On:
            state = Led0On;
            break;
        case TurnLed1Off:
            state = Led1Off;
            break;
        case TurnLed1On:
            state = Led1On;
            break;
        default:
            state = Idle;
            break;
    }
}

