# RFID Voting System

A PIC16F877A-based RFID voting system built in MPLAB X using the XC8 compiler. The system uses RFID cards to identify voters, a keypad to enter voting choices, and an I2C LCD to display messages, voting prompts, and results.

The purpose of the project is to allow only authorized users to vote, prevent the same card from voting more than once, and keep the voting results saved even if the system loses power.

## Hardware Used

- PIC16F877A microcontroller
- MFRC522 RFID reader module
- 4x4 matrix keypad
- 16x2 LCD with I2C backpack
- PICkit 3 programmer

## Ports Used

- PORTB is used for the 4x4 keypad.
- PORTC is used for the LCD I2C connection and most RFID control/SPI signals.
- PORTD is used for the RFID MISO signal.
- Internal EEPROM is used to save vote counts and voter status after power off.

## Interfaces Used

- I2C is used for the LCD.
- Software SPI is used for the MFRC522 RFID reader.
- GPIO keypad scanning is used for the 4x4 keypad.
- Internal EEPROM is used as nonvolatile memory.

## Project Behavior

When the system starts, the LCD displays a message asking the user to scan a card. The RFID reader waits for a card and reads the card UID when one is detected.

The UID is compared with the list of authorized cards stored in the program. If the card is not authorized, the LCD displays an access denied message. If the card is authorized but has already voted, the system displays that the user has already voted and does not allow another vote.

If the card is authorized and has not voted yet, the system allows the user to vote using the keypad. The voter can select candidate A, candidate B, or candidate C. After a valid vote is entered, the selected candidate vote count is increased, the card is marked as already voted, and the updated data is saved to EEPROM.

The system can also display the current vote results on the LCD. A reset option is included to clear the saved vote counts and voter status.



## Voting Logic

1. System starts and initializes the LCD, keypad, RFID reader, and saved state.
2. LCD asks the user to scan an RFID card.
3. RFID reader checks for a card.
4. Card UID is read.
5. UID is compared with authorized card UIDs.
6. Unauthorized cards are rejected.
7. Authorized cards are checked to see if they already voted.
8. If the card has not voted, the voter is allowed to choose A, B, or C.
9. Valid vote input increases the correct candidate count.
10. The card is marked as already voted.
11. Vote totals and voter status are saved in EEPROM.
12. The system returns to the scan screen for the next card.

## Keypad Use

- A/B/C are used for voting.
- Other keys are used for navigation, going back, showing results, or resetting depending on the current screen.

Invalid input is handled by displaying an error message on the LCD and waiting for the user to press another key before returning.

## Memory

The project uses the internal EEPROM of the PIC16F877A. This allows the system to remember:

- Candidate A vote count
- Candidate B vote count
- Candidate C vote count
- Which authorized cards have already voted

This means the results are not lost when the circuit is powered off.

## Source Files

- `src/main.c` initializes the system and starts the main program flow.
- `src/lcd.c` and `src/lcd.h` handle the LCD over I2C.
- `src/keypad.c` and `src/keypad.h` handle keypad scanning.
- `src/rfid.c` and `src/rfid.h` handle RFID communication, authorized cards, vote counts, and EEPROM storage.
- `src/state.c` and `src/state.h` handle the voting screens and main system logic.

