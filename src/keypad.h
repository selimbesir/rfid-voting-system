#ifndef KEYPAD_H
#define KEYPAD_H

#define _XTAL_FREQ 4000000

// Wiring: rows = RB4..RB7 (output), cols = RB0..RB3 (input)
// Physical layout:
//   Row0: 1  2  3  A
//   Row1: 4  5  6  B
//   Row2: 7  8  9  C
//   Row3: *  0  #  D
//
// System mapping:
//   '1' → 'A'  (vote A)
//   '2' → 'B'  (vote B)
//   '3' → 'C'  (vote C)
//   '#' → 'R'  (results)
//   '*' → 'X'  (reset)

char getKey(void);

#endif