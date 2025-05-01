//
// Created by Brayden Rudisill on 4/28/25.
//

#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "lcd.h"

const unsigned short OverflowsPerTick = XTAL_FRQ / 256;  // 31250.
volatile unsigned short OverflowCount = 0;

typedef struct {
    int year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} DateTime;

DateTime DT;

void InitDT() {
    DT.year = 2025;
    DT.month = 0;
    DT.day = 0;
    DT.hour = 0;
    DT.minute = 0;
    DT.second = 0;
}

char* Months[12] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

unsigned char DaysInMonths[12] = {
    31,
    28,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31
};

inline bool year_div(unsigned short n) {
    return DT.year % n == 0;
}

bool IsLeapYear() {
    return (year_div(4) && !year_div(100)) || year_div(400);
}

void TickDT() {
    ++DT.second;
    if (DT.second >= 60) {
        DT.second = 0;
        ++DT.minute;
    }
    if (DT.minute >= 60) {
        DT.minute = 0;
        ++DT.hour;
    }
    if (DT.hour >= 24) {
        DT.hour = 0;
        ++DT.day;
    }
    if (DT.day >= DaysInMonths[DT.month] || (IsLeapYear() && DT.month == 1 && DT.day >= 29)) {
        DT.day = 0;
        ++DT.month;
    }
    if (DT.month >= 12) {
        DT.month = 0;
        ++DT.year;
    }
}

ISR(TIMER0_OVF_vect) {
    if (++OverflowCount >= OverflowsPerTick) {
        TickDT();
        OverflowCount = 0;
    }
}

void TimerSet() {
    TCCR0 = 1;   // Disable prescaling
    TCNT0 = 0;   // Set initial count to 0
    TIMSK |= 1;  // Timer0 overflow interrupt enable
    sei();       // Global interrupt enable
}

void GetKey(unsigned const char index) {

}

enum OP_States { OP_Edit, OP_Display } OP_State;
enum CT_States { CT_AMPM, CT_24hr } CT_State;

void UpdateOperationMode() {

}

bool IsPressed(const unsigned char row, const unsigned char column) {
    const unsigned char column_port = column + 4;  // DDRC is [r1, r2, r3, r4, c1, c2, c3, c4]
    DDRC = 0x00;
    PORTC = 0x00;

    SET_BIT(DDRC, row);
    CLR_BIT(PORTC, row);
    // 000(0)_000(1)

    CLR_BIT(DDRC, column_port);
    SET_BIT(PORTC, column_port);

    avr_wait(5);
    // 000(1)_000(0)
    return GET_BIT(PINC, column_port) == 0;
}

void UpdateClockType() {
    switch(CT_State) {
        case CT_AMPM:
            if (IsPressed(0, 3))
                 CT_State = CT_24hr;
            break;
        case CT_24hr:
            if (IsPressed(1, 3))
                CT_State = CT_AMPM;
            break;
    }
}

void UpdateDisplay() {
    char buf[17];
    // Print date on top row.
    lcd_pos(0, 0);
    sprintf(buf, "%02d %s %04d", DT.day + 1, Months[DT.month], DT.year);
    lcd_puts2(buf);
    // Do similar thing to print time on bottom row.
}

void Update() {
    UpdateOperationMode();
    UpdateClockType();
    UpdateDisplay();
    if (IsPressed(0,0))
        SET_BIT(PORTB, 0);
}

int main(void) {
    while(1) {
        DDRB = 0xFF;
        if (IsPressed(0, 0))
            PORTB = 0x00;
        else
            PORTB = 0xFF;
    }
    return 0;
}
