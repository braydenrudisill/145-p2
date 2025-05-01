/**
 * avr.h
 * Copyright (C) 2001-2020, Tony Givargis
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))

#define WDR() asm volatile("wdr"::)
#define NOP() asm volatile("nop"::)

void avr_init();
void avr_wait(unsigned short msec);

#endif /* _UTILS_H_ */
