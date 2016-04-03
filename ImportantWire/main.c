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

// procedura reset
void ow_reset(void)
{
    CLR_DQ; // stan niski na linii 1wire
    
    _delay_us(480); // opóŸnienie ok 480us

    SET_DQ;// stan wysoki na linii 1wire
    
    _delay_us(480); // opóŸnienie ok 480 us
}
// procedura zapisu bitu na liniê 1wire
void ow_write_bit(char b)
{
    CLR_DQ; // stan niski na linii 1wire
    _delay_us(10); // opóŸnienie 10us
    if(b) SET_DQ; // jeœli parametr jest niezerowy to ustaw stan wysoki na linii
    _delay_us(100); // opóŸnienie 100us
    SET_DQ; // stan wysoki na linii 1wire
}

char ow_read_bit(void)
{
    CLR_DQ;	// stan niski na linii 1Wire
    _delay_us(3);	// opoznienie 3us
    SET_DQ;	// stan wysoki na linii 1Wire
    _delay_us(15); // opoznienie 15us
    if(IN_DQ) return 1; else return 0; // testowanie linii, funkcja zwraca stan
}

unsigned char ow_read_byte(void)
{
    unsigned char i; // iterator pêtli
    unsigned char value = 0; // odczytany bajt
    for (i=0;i<8;i++){ // odczyt 8 bitów z magistrali DQ
        if(ow_read_bit()) value|=0x01<<i;
        _delay_us(9); // opóŸnienie 9us
    }
    return(value);
}

void ow_write_byte(char val)
{
    unsigned char i; // iterator pêtli
    unsigned char temp;
    for (i=0; i<8; i++){ // wys³anie 8 bitów na magistralê 1Wire
        temp = val >> i;
        temp &= 0x01;
        ow_write_bit(temp);
    }
    _delay_us(9);
}

/* W tablicy bêd¹ formowane komunikaty tekstowe
   wysy³ane do wyœwietlacza */
char str[17]="                 ";
double temperature=0.0;
double t = 0.0;
char scratchpad[9];
void odczyt_temperatury(){
    uint8_t i;
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
    ow_reset();			 // reset 1Wire
    ow_write_byte(0xCC); // skip ROM
    ow_write_byte(0x44); // convertT
    STRONG_PULL_UP_ON;
    _delay_ms(750);		// delay 750ms
    //_delay_ms(250);
//	_delay_ms(250);
    STRONG_PULL_UP_OFF;
    ow_reset();			 // reset 1Wire
    ow_write_byte(0xCC); // skip ROM
    ow_write_byte(0xBE); // read Scratchpad
    for(i=0; i<9; i++)	 
        scratchpad[i] = ow_read_byte();

    uint16_t buffer = scratchpad[0];
    buffer |= (scratchpad[1] << 8);
    temperature = (double)(buffer / 16.0);

    dtostrf(temperature, 0,4, str); // temperature to string
    str[15] = '\r';
    str[16] = '\n';
    UsartWrite(str); // wys³anie temperatury
}

volatile char data;

int main(void)
{
    DDRC |= (1 << PC1);
    USART0Init();
    
    // initialize interrupts
    sei();
    
    while(1){
        switch(data){
            case 't':
               // cli();
                PORTC &= ~(1 << 1);
                odczyt_temperatury();
                UsartWrite("temperatura!");
               // sei();
                break;
            default:
                break;
        }
    }

    return 0;
}

// onUsart
ISR(USART0_RXC_vect)
{
    data = UDR0;
}
