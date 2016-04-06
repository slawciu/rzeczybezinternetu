#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdlib.h>

// 1WIRE CONFIG
#define DQ 2 // pin DQ termometru DS1822-PAR
#define THERM_DDRx DDRD
#define THERM_PINx PIND

#define STRONG_PULL_UP_DDR DDRC
#define STRONG_PULL_UP_PORT PORTC
#define STRONG_PULL_UP_PIN 1

#define SET_DQ THERM_DDRx &= ~(1 << DQ) // linia DQ w stan wysoki
#define CLR_DQ THERM_DDRx |= (1 << DQ)	// linia DQ w stan niski
#define IN_DQ THERM_PINx & (1 << DQ)	// sprawdzenie stanu linii DQ (odczyt)

#define STRONG_PULL_UP_ON STRONG_PULL_UP_PORT |= (1 << STRONG_PULL_UP_PIN)
#define STRONG_PULL_UP_OFF STRONG_PULL_UP_PORT &= ~(1 << STRONG_PULL_UP_PIN)

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

void UsartWrite(uint8_t* text)
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

void OneWireReset(void)
{
    CLR_DQ; // stan niski na linii 1wire
    
    _delay_us(480); // opoznienie ok 480us

    SET_DQ;// stan wysoki na linii 1wire
    
    _delay_us(480); // opoznienie ok 480 us
}

void OneWireWriteBit(uint8_t bit)
{
    CLR_DQ; // stan niski na linii 1wire
    _delay_us(10); // opoznienie 10us
    if(bit)
	{ 
		SET_DQ; // jezeli parametr jest niezerowy to ustaw stan wysoki na linii
	}
    _delay_us(100); // opoznienie 100us
    SET_DQ; // stan wysoki na linii 1wire
}

uint8_t OneWireReadBit(void)
{
    CLR_DQ;	// stan niski na linii 1Wire
    _delay_us(3);	// opoznienie 3us
    SET_DQ;	// stan wysoki na linii 1Wire
    _delay_us(15); // opoznienie 15us
    if(IN_DQ)
	{ 
		return 1;
	}
	else
	{
		return 0; // testowanie linii, funkcja zwraca stan
	}
}

uint8_t OneWireReadByte(void)
{
    uint8_t i; // iterator petli
    uint8_t value = 0; // odczytany bajt
    for (i=0;i<8;i++)
	{ 
		// odczyt 8 bitów z magistrali DQ
        if(OneWireReadBit())
		{
			value|=0x01<<i;
		}
        _delay_us(9); // opoznienie 9us
    }
    return(value);
}

void OneWireWriteByte(uint8_t val)
{
    uint8_t i; // iterator petli
    uint8_t temp;
    for (i=0; i<8; i++)
	{
		// wyslanie 8 bitów na magistrale 1-Wire
        temp = val >> i;
        temp &= 0x01;
        OneWireWriteBit(temp);
    }
    _delay_us(9);
}

void ReadTemperature(){
    uint8_t i;
	double temperature=0.0;
	uint8_t str[17]="                 ";
	uint8_t scratchpad[9];
    /*
        stratchpad[]:
        0 - lsb
        1 - msb
        2 - T_{h} register
        3 - T_{l} register
        4 - configuartion register
        5 - reserved
        6 - reserved
        7 - reserved
        8 - crc
    */
    STRONG_PULL_UP_OFF;
    OneWireReset();			 // reset 1-Wire
    OneWireWriteByte(0xCC); // komenda skip ROM
    OneWireWriteByte(0x44); // komenda convertT
    STRONG_PULL_UP_ON;
    _delay_ms(750);		// delay 750ms
    STRONG_PULL_UP_OFF;
    OneWireReset();			 // reset 1Wire
    OneWireWriteByte(0xCC); // komenda skip ROM
    OneWireWriteByte(0xBE); // komenda read Scratchpad
    
	for(i=0; i<9; i++)	 
    {
        scratchpad[i] = OneWireReadByte();
    }

    uint16_t buffer = scratchpad[0];
    buffer |= (scratchpad[1] << 8);
    temperature = (double)(buffer / 16.0);

    dtostrf(temperature, 0,4, str); // konwersja do napisu
    str[15] = '\r';
    str[16] = '\n';
    UsartWrite(str); // wyslanie temperatury
}

volatile uint8_t command;

typedef enum
{
    Idle = 0,
    MeasurementOn
} States;

uint8_t state = Idle;
int main(void)
{
    STRONG_PULL_UP_DDR |= (1 << STRONG_PULL_UP_PIN);
    USART0Init();
    
    // initialize interrupts
    sei();
    
    while(1){
        switch(data){
            case 'T':
                state = MeasurementOn;        
                break;
            case 't':
                state = Idle;
                break;
            default:
                state = Idle;
                break;
        }
        
        if (state == MeasurementOn)
        {
            PORTC &= ~(1 << 1);
            odczyt_temperatury();
            _delay_ms(500);
        }
        else
        {
            
        }
    }

    return 0;
}

// onUsart
ISR(USART0_RXC_vect)
{
    data = UDR0;
}
