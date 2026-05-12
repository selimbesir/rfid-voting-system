# RFID Voting System

PIC16F877A voting system using an MFRC522 RFID reader, 4x4 keypad, and 16x2 I2C LCD.

## Hardware Interfaces

- I2C LCD on RC3/SCL and RC4/SDA
- MFRC522 RFID reader using software SPI on RC1, RC2, RC6, RC7, and RD0
- 4x4 keypad matrix on RB0-RB7
- PIC internal EEPROM for nonvolatile vote and voter-status storage

## Behavior

Authorized RFID cards can vote once for candidate A, B, or C using the keypad. Results can be displayed on the LCD, and reset mode clears stored votes and voter flags.
