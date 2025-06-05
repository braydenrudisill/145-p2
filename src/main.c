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

const unsigned short OverflowsPerTick = XTAL_FRQ / 256 / 256;
volatile unsigned short OverflowCount = 0;
volatile unsigned short EditIndex = 0;
volatile bool IsHoldingButton = false;
volatile unsigned char UnconfirmedEdits[14] = {
    0,0,0,0,  // Year
    0,0,      // Month
    0,0,      // Day
    0,0,      // Hour
    0,0,      // Minute
    0,0       // Second
};

typedef struct {
    volatile int year;
    volatile unsigned char month;
    volatile unsigned char day;
    volatile unsigned char hour;
    volatile unsigned char minute;
    volatile unsigned char second;
} DateTime;

volatile DateTime* DT;

void InitDT() {
    DT = (DateTime*) malloc(sizeof(DateTime));
    DT->year = 2025;
    DT->month = 0;
    DT->day = 0;
    DT->hour = 0;
    DT->minute = 0;
    DT->second = 0;
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

bool year_div(const unsigned short n) {
    return DT->year % n == 0;
}

bool IsLeapYear() {
    return (year_div(4) && !year_div(100)) || year_div(400);
}

void TimerSet() {
    TCCR0 = 4;   // Disable prescaling
    TCNT0 = 0;   // Set initial count to 0
    TIMSK |= 1;  // Timer0 overflow interrupt enable
    sei();       // Global interrupt enable
}

enum OP_States { OP_Edit, OP_Display } OP_State = OP_Display;
enum CT_States { CT_AMPM, CT_24hr } CT_State = CT_24hr;

void TickDT() {
    if (OP_State != OP_Display)
        return;
    if (DT==NULL)
        return;

    ++DT->second;
    if (DT->second >= 60) {
        DT->second = 0;
        ++DT->minute;
    }
    if (DT->minute >= 60) {
        DT->minute = 0;
        ++DT->hour;
    }
    if (DT->hour >= 24) {
        DT->hour = 0;
        ++DT->day;
    }
    if ((IsLeapYear() && DT->month == 1 && DT->day >= 29) || ((!IsLeapYear() || DT->month != 1) && DT->day >= DaysInMonths[DT->month])) {
        DT->day = 0;
        ++DT->month;
    }
    if (DT->month >= 12) {
        DT->month = 0;
        ++DT->year;
    }
}

ISR(TIMER0_OVF_vect) {
    if (++OverflowCount >= OverflowsPerTick) {
        TickDT();
        OverflowCount = 0;
    }
}


// PC is [r1, r2, r3, r4, c1, c2, c3, c4]
bool IsPressed(const unsigned char row, const unsigned char column) {
    const unsigned char column_port = column + 4;
    DDRC = 0x00;
    PORTC = 0x00;

    SET_BIT(DDRC, row);
    CLR_BIT(PORTC, row);
    // 000(0)_000(1)

    CLR_BIT(DDRC, column_port);
    SET_BIT(PORTC, column_port);

    for (int i = 0; i < 2000; ++i)
        NOP();

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

void UpdateOperationMode() {
    switch(OP_State) {
        case OP_Display:
            if (IsPressed(2, 3)) {
                OP_State = OP_Edit;
                lcd_clr();
                EditIndex = 0;
            }
        break;
        case OP_Edit:
            if (IsPressed(3, 3) && EditIndex==14) {
                OP_State = OP_Display;
                DT->month = 10 * UnconfirmedEdits[0] + UnconfirmedEdits[1] - 1;
                DT->day = 10 * UnconfirmedEdits[2] + UnconfirmedEdits[3] - 1;
                DT->year = 1000 * UnconfirmedEdits[4] + 100 * UnconfirmedEdits[5] + 10 * UnconfirmedEdits[6] + UnconfirmedEdits[7];
                DT->hour = 10 * UnconfirmedEdits[8] + UnconfirmedEdits[9];
                DT->minute = 10 * UnconfirmedEdits[10] + UnconfirmedEdits[11];
                DT->second = 10 * UnconfirmedEdits[12] + UnconfirmedEdits[13];
            }
        break;
    }
}
int GetNumberPressed() {
    int r, c;
    if (IsPressed(3, 1))
        return 0;

    for (r = 0; r < 3; ++r)
        for (c = 0; c < 3; ++c)
            if (IsPressed(r, c))
                return 3 * r + c + 1;

    return -1;
}
void HandleEdits() {
    SET_BIT(DDRB, 4);
    if (EditIndex % 2 == 0)
        SET_BIT(PORTB, 4);
    else
        CLR_BIT(PORTB, 4);

    int n = GetNumberPressed();

    if (n == -1) {
        IsHoldingButton = false;
        return;
    }

    if(!IsHoldingButton && EditIndex < 14) {
        UnconfirmedEdits[EditIndex++] = n;
        IsHoldingButton = true;
        lcd_pos(0,0);
        for (int i = 0; i < EditIndex; ++i) {
            char buf[1];
            sprintf(buf, "%1d", UnconfirmedEdits[i]);
            lcd_puts2(buf);
        }
    }

}

void UpdateDisplay() {
    char buf[17];

    // Print date on top row.
    lcd_pos(0, 0);
    sprintf(buf, "%02d/%02d/%04d      ", DT->month + 1, DT->day + 1, DT->year);
    lcd_puts2(buf);

    // Do similar thing to print time on bottom row.
    if (CT_State == CT_24hr) {
        lcd_pos(1, 0);
        sprintf(buf, "%02d:%02d:%02d      ", DT->hour, DT->minute, DT->second);
        lcd_puts2(buf);
    }
    else {
        lcd_pos(1, 0);
        sprintf(buf, "%02d:%02d:%02d %s", DT->hour == 0 ? 12 : ((char) DT->hour - 1) % 12 + 1, DT->minute, DT->second, DT->hour < 12 ? "AM" : "PM");
        lcd_puts2(buf);
    }
}

void Update() {
    wdt_reset();
    lcd_pos(1, 1);
    lcd_puts2("hiii!  ");
    SET_BIT(DDRB, 4);
    SET_BIT(PORTB, 4);
    // UpdateOperationMode();
    // UpdateClockType();
    // if (OP_State == OP_Edit)
    //     HandleEdits();
    // else if (OP_State == OP_Display)
    //     UpdateDisplay();
}

int main(void) {
    avr_init();
    lcd_init();
    // lcd_pos(1, 1);
    // lcd_puts2("hiii!  ");
    // InitDT();
    // TimerSet();

    while(1) Update();
    return 0;
}
