#include <xc.h>
#include "lcd.h"

// ===================== I2C =====================

void I2C_Init() {
    TRISC3 = 1; TRISC4 = 1;
    SSPCON  = 0x28;
    SSPADD  = 9;
    SSPSTAT = 0x00;
}

void I2C_Wait() {
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
}

void LCD_Write_I2C(unsigned char data) {
    SSPCON2bits.SEN = 1; while (SSPCON2bits.SEN);
    SSPBUF = (I2C_ADDR << 1); I2C_Wait();
    SSPBUF = data;            I2C_Wait();
    SSPCON2bits.PEN = 1; while (SSPCON2bits.PEN);
}

// ===================== LCD =====================

void LCD_Cmd_Data(unsigned char val, unsigned char mode) {
    unsigned char high = (val & 0xF0)        | mode | 0x08;
    unsigned char low  = ((val << 4) & 0xF0) | mode | 0x08;
    LCD_Write_I2C(high | 0x04); __delay_us(50); LCD_Write_I2C(high & ~0x04);
    LCD_Write_I2C(low  | 0x04); __delay_us(50); LCD_Write_I2C(low  & ~0x04);
    __delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col) {
    unsigned char addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_Cmd_Data(addr, 0);
}

void LCD_Clear() {
    LCD_Cmd_Data(0x01, 0);
    __delay_ms(2);
}

void LCD_Init() {
    __delay_ms(50);
    LCD_Cmd_Data(0x33, 0);
    LCD_Cmd_Data(0x32, 0);
    LCD_Cmd_Data(0x28, 0);
    LCD_Cmd_Data(0x0C, 0);
    LCD_Cmd_Data(0x01, 0);
    __delay_ms(2);
}

void LCD_Print(const char *str) {
    while (*str) LCD_Cmd_Data(*str++, 1);
}

void LCD_PrintInt(int val) {
    char buf[8];
    unsigned char i = 0;
    if (val == 0) { LCD_Cmd_Data('0', 1); return; }
    while (val > 0) { buf[i++] = '0' + (unsigned char)(val % 10); val /= 10; }
    while (i > 0)   LCD_Cmd_Data(buf[--i], 1);
}