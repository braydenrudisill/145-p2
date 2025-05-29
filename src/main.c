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

enum States { Playing, Paused } State;
bool IsPressed;

typedef enum {
    A, As, B, C, Cs, D, Ds, E, F, Fs, G, Gs
} Note;

typedef enum {
    Whole, Half, Quarter, Eighth
} Duration;

typedef struct {
    Note note;
    Duration duration;
} PlayingNote;

PlayingNote song[] = {
    {E, Quarter},    // Mary
    {D, Quarter},    // had
    {C, Quarter},    // a
    {D, Quarter},    // lit-
    {E, Quarter},    // tle
    {E, Quarter},    // lamb
    {E, Half},       // (hold)

    {D, Quarter},    // lit-
    {D, Quarter},    // tle
    {D, Half},       // lamb

    {E, Quarter},    // lit-
    {G, Quarter},    // tle
    {G, Half},       // lamb

    {E, Quarter},    // Mary
    {D, Quarter},    // had
    {C, Quarter},    // a
    {D, Quarter},    // lit-
    {E, Quarter},    // tle
    {E, Quarter},    // lamb
    {E, Half},       // (hold)

    {E, Quarter},    // Its
    {D, Quarter},    // fleece
    {D, Quarter},    // was
    {E, Quarter},    // white
    {D, Quarter},    // as
    {C, Whole}       // snow
};


int lengths[] = {
    400,
    200,
    100,
    50,
};

int wait_times[] = {
    142,
    134,
    127,
    120,
    113,
    106,
    100,
    95,
    90,
    85,
    80,
    75,
};

unsigned long CurrentNote;

void PlayNextNote() {
    if (CurrentNote >= sizeof(song) / sizeof(song[0]))
        return;

    PlayingNote note = song[CurrentNote++];

    for (int i = 0; i < lengths[note.duration] * 3; ++i) {
        SET_BIT(PORTC, 0);
        avr_wait(wait_times[note.note]);
        CLR_BIT(PORTC, 0);
        avr_wait(wait_times[note.note]);
    }
}

void Update() {
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