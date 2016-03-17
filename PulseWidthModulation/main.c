#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void Wait()
{
    _delay_ms(4);
}

int main(void)
{
    uint8_t brightness = 0;
    uint8_t maxBrightness = 250;
    uint8_t minBrightness = 0;
    
    // Fast PWM mode
    TCCR0 |= (1 << WGM01) | (1 << WGM00);
    
    // OC0 enabled, clear on match
    TCCR0 |= (1 << COM01) | (1 << COM00);
    
    // timer0 clock source prescaler
    TCCR0 |= (1 << CS00);
    
    // pin OC0 as output
    DDRB |= (1 << PB0);
    
    while (1)
    {
        for (brightness = minBrightness; brightness < maxBrightness; brightness++)
        {
            OCR0 = brightness;
            Wait();
        }
        
        for (brightness = maxBrightness; brightness > minBrightness; brightness--)
        {
            OCR0 = brightness;
            Wait();
        }
    }
}

