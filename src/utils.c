/**
 * avr.c
 * Copyright (C) 2001-2020, Tony Givargis
 */

#include "utils.h"
#include <avr/io.h>

void avr_init()
{
	WDTCR = 15;
}

void avr_wait(unsigned short usec_8s) {
	TCCR0 = 3;

	TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.000008 * usec_8s);
	SET_BIT(TIFR, TOV0);
	while (!GET_BIT(TIFR, TOV0));
	TCCR0 = 0;
}
