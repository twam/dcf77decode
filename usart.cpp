#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "usart.h"
#include "fifo.h"

// c++ workaround
FILE usartFile = {NULL, 0, _FDEV_SETUP_RW, 0, 0, usartPutchar, usartGetchar, 0};
//FILE usartFile = FDEV_SETUP_STREAM(usartPutchar, usartGetchar, _FDEV_SETUP_RW);

volatile uint8_t usartLineReceived;

uint8_t usartTxBuffer[USART_TXBUFFER];
uint8_t usartRxBuffer[USART_RXBUFFER];

fifo_t usartTxFifo;
fifo_t usartRxFifo;

// send a char on line
int usartPutchar(char c, FILE* f)
{
	uint8_t ret = 0;
	while (!ret) {
		ret= fifo_put(&usartTxFifo, c);
	}
	UCSR0B |= _BV(UDRIE0);
	return ret;
}

// get a char from line
int usartGetchar(FILE* f)
{
	return fifo_get_wait (&usartRxFifo);
}

uint8_t usartRXCount() {
	return usartRxFifo.count;
}

// initialize usart
void usartInit(void) {

	// save status register
	uint8_t sreg = SREG;

	// disable interrupts temporary
	cli();

	UBRR0 = USART_BAUD_CALC(USART_BAUD_RATE,F_CPU);

	UCSR0A |= _BV(U2X0);
    UCSR0B |= _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);

	// flush buffer und clear transmit/receive complete flags
	do { UDR0; } while (UCSR0A & _BV(RXC0));
	UCSR0A |= _BV(RXC0) | _BV(TXC0);

	// initialize fifo
	fifo_init(&usartTxFifo,usartTxBuffer, USART_TXBUFFER);
	fifo_init(&usartRxFifo,usartRxBuffer, USART_RXBUFFER);

	// set usart to stdout
	stdout = &usartFile;
	stdin = &usartFile;

	// restore status register
	SREG = sreg;

	usartLineReceived = 0;
}

// handle incoming char on line
ISR(USART0_RX_vect)
{
	uint8_t data = UDR0;
	_inline_fifo_put (&usartRxFifo, data);

	if (data == '\n')
		usartLineReceived = 1;
}

// handle send complete on line
ISR(USART0_UDRE_vect)
{
	if (usartTxFifo.count > 0)
		UDR0 = _inline_fifo_get (&usartTxFifo);
	else
		UCSR0B &= ~_BV(UDRIE0);
}

