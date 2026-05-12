#pragma config FOSC = XT, WDTE = OFF, PWRTE = ON, BOREN = OFF, LVP = OFF

#include <xc.h>
#include "lcd.h"
#include "rfid.h"
#include "keypad.h"
#include "state.h"

void main() {
    ADCON1 = 0x06;
    OPTION_REGbits.nRBPU = 0; // Enable PORTB weak pull-ups for keypad inputs.

    // SPI pins
    TRISCbits.TRISC1 = 0; // RST  – output
    TRISCbits.TRISC2 = 0; // CS   – output
    TRISCbits.TRISC6 = 0; // SCK  – output
    TRISCbits.TRISC7 = 0; // MOSI – output
    TRISDbits.TRISD0 = 1; // MISO – input

    RFID_CS  = 1;
    RFID_SCK = 0;

    I2C_Init();
    LCD_Init();
    load_saved_state();
    MFRC522_Init();

    while (1) {
        State_Run();
    }
}