#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <string.h>

#define UBRR_VALUE_GSM 25
#define BUFFER_SIZE 300

volatile uint8_t indexBuffer = 0;
volatile char gsmBuffer[BUFFER_SIZE];
volatile uint8_t messageReceived = 0;
volatile uint8_t applicationState;

typedef enum 
{
	Idle,
	GSM_Init,
	GSM_WaitForAte,
	SMS_SendInit,
	SMS_WaitForTypeMessage,
	SMS_WaitForMessageSend,
	SMS_ReadInit,
	GSM_ReadWaitForAte,
	GSM_Error,
	GSM_WaitForCmgfResponse,
	GSM_WaitForCpmsResponse,
	GSM_WaitForSMS
} ApplicationStates;

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

void ClearBuffer()
{
	int i;
	for (i = indexBuffer; i >= 0; i--)
	{
		gsmBuffer[i] = '\0';
	}
	indexBuffer = 0;
}

void InitMessageSend()
{
	ClearBuffer();
	UsartFlush();
	UsartWrite("\r\n");
	_delay_ms(10);
	UsartWrite("AT+CMGF=1\r\n");
	applicationState = SMS_SendInit;
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
    char* readPointer;
    
    DDRC |= (1 << PC0); // GSM power line
    USART0Init();
    sei();
    TurnGSMOn();
    
    _delay_ms(15000);
    UsartFlush();

	applicationState = Idle;
        
    while (1)
    {
	    if (messageReceived == 1 || applicationState == SMS_ReadInit || applicationState == GSM_Init)
	    {
		    switch (applicationState)
		    {
			    case GSM_Init:
					while(1)
					{
						if (messageReceived == 1)
						{
							if (strstr(gsmBuffer, "OK") != NULL)
							{
								ClearBuffer();
								messageReceived = 0;
							}
							else
							{
								_delay_ms(200);
							}
							break;
						}
						UsartFlush();
						ClearBuffer();
						_delay_ms(10);
						UsartWrite("AT\r\n");
						_delay_ms(1000);
					}
					UsartFlush();
					ClearBuffer();
					UsartWrite("ATE000\r\n");
					applicationState = GSM_WaitForAte;
					break;
			    case GSM_WaitForAte:
					UsartFlush();
					ClearBuffer();
					_delay_ms(10);
					UsartWrite("AT+CMGF=1\r\n");
					applicationState = SMS_SendInit;
					break;
			    case SMS_SendInit:
					if (strstr(gsmBuffer, "OK") != NULL)
					{
						cli();
						ClearBuffer();
						UsartWrite("AT+CMGS=\"");
						UsartWrite("000000000"); // phone number here
						sei();
						UsartWrite("\"\r\n");
				    
						applicationState = SMS_WaitForTypeMessage;
					}
					else
					{
						applicationState = Idle;
						InitMessageSend();
					}
					break;
			    case SMS_WaitForTypeMessage:
					if (strstr(gsmBuffer, ">") != NULL)
					{
						ClearBuffer();
						UsartFlush();
						UsartWrite("Daj sie poznac!"); // message here
				    				    
						_delay_ms(200);
						UsartWrite("\x1A\r\n");
						_delay_ms(200);
						applicationState = SMS_WaitForMessageSend;
					}
					else
					{
						applicationState = Idle;
						InitMessageSend();
					}
					break;
			    case SMS_WaitForMessageSend:
					if (strstr(gsmBuffer,"+CMGS:") != NULL)
					{
						applicationState = Idle;
						_delay_ms(1000);
						ClearBuffer();
					}
					else
					{
						ClearBuffer();
						applicationState = Idle;
						InitMessageSend();
					}
					break;
				case GSM_Error:
					applicationState = Idle;
					break;
			    case SMS_ReadInit:
					while(1)
					{
						if (messageReceived == 1)
						{
							if (strstr(gsmBuffer, "OK") != NULL)
							{
								ClearBuffer();
								messageReceived = 0;
							}
							else
							{
								_delay_ms(200);
							}
							break;
						}
						UsartFlush();
						ClearBuffer();
						UsartWrite("\r\n");
						_delay_ms(10);
						UsartWrite("AT\r\n");
						_delay_ms(1000);
					}
					UsartFlush();
					ClearBuffer();
					UsartWrite("ATE000\r\n");
					applicationState = GSM_ReadWaitForAte;
					break;
			    case GSM_ReadWaitForAte:
					ClearBuffer();
					UsartFlush();
					UsartWrite("AT+CMGF=1\r\n");
					applicationState = GSM_WaitForCmgfResponse;
					break;
			    case GSM_WaitForCmgfResponse:
					ClearBuffer();
					UsartFlush();
					UsartWrite("AT+CPMS=\"SM\"\r\n");
					applicationState = GSM_WaitForCpmsResponse;
					break;
			    case GSM_WaitForCpmsResponse:
					ClearBuffer();
					UsartFlush();
					UsartWrite("AT+MMGR=1\r\n");
					applicationState = GSM_WaitForSMS;
					break;
			    case GSM_WaitForSMS:
					if (strstr(gsmBuffer,"ERROR") == NULL)
					{
						// we handle '[a' and '[A' messages only
						readPointer = strchr(gsmBuffer, '[');
						if (readPointer == NULL)
						{
							// delete all messages, 'cause are not ours
							ClearBuffer();
							UsartFlush();
							UsartWrite("AT+CMGD=1,4\r\n");
					    
							applicationState = Idle;
							break;
						}
				    
						readPointer++;
				    
						switch(*readPointer)
						{
							case 'A':
								// action here
								break;
							case 'a':
								// action here
								break;
							default:
								ClearBuffer();
								UsartFlush();
								applicationState = Idle;
								break;
						}
					
						UsartWrite("AT+CMGD=1,4\r\n");
					}
					else
					{
						applicationState = Idle;
					}
					ClearBuffer();
					break;
			    
		    }
		    messageReceived = 0;
	    }
    }
}

// onUsart
ISR(USART0_RXC_vect)
{
	if (indexBuffer > BUFFER_SIZE)
	{
		indexBuffer = 0;
	}
	
	gsmBuffer[indexBuffer] = UDR0;


	if (indexBuffer > 0)
	{
		switch (applicationState)
		{
			case GSM_ReadWaitForAte:
				if (strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case GSM_WaitForAte:
				if (strstr(gsmBuffer, "AT") != NULL || strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case SMS_ReadInit:
				if (strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case SMS_SendInit:
				if (strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case SMS_WaitForTypeMessage:
				if (strstr(gsmBuffer, ">") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case SMS_WaitForMessageSend:
				if (strstr(gsmBuffer,"+CMGS:") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case Idle:
				if (strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case GSM_WaitForCmgfResponse:
				if (strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
			case GSM_WaitForCpmsResponse:
				if (strstr(gsmBuffer, ",20\r\n") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
			case GSM_WaitForSMS:
				if (strstr(gsmBuffer, "+MMGR:") != NULL && strstr(gsmBuffer, "OK") != NULL || strstr(gsmBuffer, "ERROR") != NULL)
				{
					messageReceived = 1;
				}
				break;
		}
	}
	indexBuffer++;
}
