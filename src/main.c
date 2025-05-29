//
// Created by Brayden Rudisill on 4/28/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "utils.h"
#include "lcd.h"

void ADC_Init() {
    DDRA = 0x00;
    ADCSRA = 0x87;
    ADMUX = 0x40;
}

int ADC_Read(char channel) {
    ADMUX = ADMUX | (channel & 0x0f);
    SET_BIT(ADCSRA, ADSC);
    while (GET_BIT(ADCSRA, 4));
    int adcl = ADCL;
    int adch = ADCH;

    return adcl  + (adch * 256);
}

int main(void) {
    char buf[20];
    avr_init();
    ADC_Init();
    lcd_init();
    while(1) {
        wdt_reset();
        sprintf(buf, "%d        ", ADC_Read(0));
        lcd_pos(0, 0);
        lcd_puts2(buf);
        avr_wait(500);
    }
    return 0;
}