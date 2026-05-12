#include <xc.h>
#include "keypad.h"
// Physical layout:
//   Row0: 1  2  3  A
//   Row1: 4  5  6  B
//   Row2: 7  8  9  C
//   Row3: *  0  #  D
//
// System mapping:
//   '1' ? 'A'  (vote A)
//   '2' ? 'B'  (vote B)
//   '3' ? 'C'  (vote C)
//   '#' ? 'R'  (results)
//   '*' ? 'X'  (reset)
//   else ? 0   (ignored)

static const char keymap[4][4] = {
    {'D', '#', '0', '*'},
    {'C', '9', '8', '7'},
    {'B', '6', '5', '4'},
    {'A', '3', '2', '1'}
};

char getKey() {
    static unsigned char keyHeld = 0;

    TRISB = 0x0F; // RB0-3 input, RB4-7 output
    PORTB = 0xFF;
 
    for (unsigned char row = 0; row < 4; row++) {
        PORTB = (0xF0 & ~(0x10 << row));
        __delay_us(10);

        for (unsigned char col = 0; col < 4; col++) {
            if (!(PORTB & (1 << col))) {
                __delay_ms(20); // debounce
                if (!(PORTB & (1 << col))) {
                    if (keyHeld) return 0;
                    keyHeld = 1;
                    PORTB = 0xFF;
                    return keymap[row][col];
                }
            }
        }
    }
    keyHeld = 0;
    PORTB = 0xFF;
    return 0;
}