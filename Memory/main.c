#include <avr/io.h>
#include <inttypes.h>
#include <avr/eeprom.h>

void EEPROM_write(unsigned int address, uint8_t data){
    
    while(EECR & (1<<EEWE)); // poczekaj na zako?czenie poprzedniego zapisu
    
    EEAR = address; // ustawienie adresu bajtu do zapisu
    EEDR = data; // dane do zapisu

    EECR |= (1<<EEMWE); // master write enable

    EECR |= (1<<EEWE); // eeprom write enable
}

uint8_t EEPROM_read(unsigned int address){

    while(EECR & (1<<EEWE)); // poczekaj na zako?czenie poprzedniego zapisu

    EEAR = address; // ustawienie adresu bajtu do odczytu

    EECR |= (1<<EERE); // eeprom read enable
    
    return EEDR; // zwrócenie odczytanego bajtu
}

int main(void)
{
    while (1) 
    {
    }
}

