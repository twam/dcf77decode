/**************************************************************************
 *   Copyright (C) 2011 by Tobias Mueller                                  *
 *   Tobias_Mueller@twam.info                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANYs WARRANTY; without even the implied warranty of       *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usart.h"
#include "dcf77_demodulator.h"
#include "dcf77_second_decoder.h"

ISR(TIMER2_COMPA_vect) {
	DCF77_Demodulator::detector(PINA & 0x01);
}

int main(void) {
	// initialize uart first (for debugging :))
	usartInit();

	DCF77_Demodulator::setup();

  DDRA &= ~0x01;


  // Timer 2 CTC mode, prescaler 64
  TCCR2B = (0<<WGM22) | (1<<CS22);
  TCCR2A = (1<<WGM21) | (0<<WGM20);

  // 249 + 1 == 250 == 250 000 / 1000 =  (16 000 000 / 64) / 1000
  OCR2A = 249;

  // enable Timer 2 interrupts
  TIMSK2 = (1<<OCIE2A);

	sei();

	while (1) {
    DCF77_Demodulator::debug();
    DCF77_Second_Decoder::debug();
		_delay_ms(10);
	}
}
