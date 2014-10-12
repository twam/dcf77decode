#ifndef _USART_H_
#define _USART_H_

#include <stdio.h>
#include <inttypes.h>

//  Copyright 2014 Tobias Müller, twam.info
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see http://www.gnu.org/licenses/

#define USART_BAUD_RATE 115200
#define USART_BAUD_CALC(USART_BAUD_RATE,F_CPU) ((F_CPU)/((USART_BAUD_RATE)*8L)-1)

#define USART_RXBUFFER 0x80
#define USART_TXBUFFER 0x80

extern FILE usartFile;
extern volatile uint8_t usartLineReceived;

void usartInit(void);
int usartPutchar(char, FILE*);
int usartGetchar(FILE*);
uint8_t usartRXCount(void);

#endif
