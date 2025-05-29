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

enum VM_States { VM_Wait, VM_Run } VM_State;

int max = 0;
int min = 1024;
int recents[100] = {0};
int idx = 0;
int total = 0;

void reset_min_max() {
    min = 1024;
    max = 0;
}

long GetAverage() {
    long sum = 0;
    for (int i = 0; i < 100; i++)
        sum += recents[i];
    return sum / 100;
}

void update_min_max(int adc_value) {
    if (adc_value > max) {
        max = adc_value;
    }
    if (adc_value < min) {
        min = adc_value;
    }
}

bool Holding = false;
void TickVM() {
    if (!Holding && GET_BIT(PINC, 0)) {
        VM_State ^= 1;
        Holding = true;
    }

    if (Holding && !GET_BIT(PINC, 0))
        Holding = false;
}

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
    char buf2[20];
    avr_init();
    ADC_Init();
    lcd_init();
    SET_BIT(DDRC, 4);
    CLR_BIT(DDRC, 0);
    while(1) {
        wdt_reset();
        TickVM();
        if (VM_State == VM_Wait)
            SET_BIT(PORTC, 4);
        else
            CLR_BIT(PORTC, 4);

        int value = ADC_Read(0);

        recents[idx] = value;
        idx++;
        idx %= 100;
        if (total < 100)
            total++;

        update_min_max(value);

        if (VM_State == VM_Run) {
            dtostrf((float) value / 1023 * 5.0, 3, 2, buf2 );
            sprintf(buf, "N: %s", buf2);
            lcd_pos(0, 0);
            lcd_puts2(buf);
            dtostrf((float) min / 1023 * 5.0, 3, 2, buf2 );
            sprintf(buf, "L: %s", buf2);
            lcd_pos(0, 8);
            lcd_puts2(buf);
            dtostrf((float) max / 1023 * 5.0, 3, 2, buf2 );
            sprintf(buf, "H: %s", buf2);
            lcd_pos(1, 0);
            lcd_puts2(buf);
            dtostrf((float) GetAverage() / 1023 * 5.0, 3, 2, buf2 );
            sprintf(buf, "A: %s", buf2);
            lcd_pos(1, 8);
            lcd_puts2(buf);
        }
        else if (VM_State == VM_Wait){
            reset_min_max();
            sprintf(buf, "N: ----");
            lcd_pos(0, 0);
            lcd_puts2(buf);
            sprintf(buf, "L: ----");
            lcd_pos(0, 8);
            lcd_puts2(buf);
            sprintf(buf, "H: ----");
            lcd_pos(1, 0);
            lcd_puts2(buf);
            sprintf(buf, "A: ----");
            lcd_pos(1, 8);
            lcd_puts2(buf);
        }
        avr_wait(500);
    }
    return 0;
}