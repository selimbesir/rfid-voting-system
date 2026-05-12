#ifndef LCD_H
#define LCD_H

#define _XTAL_FREQ 4000000
#define I2C_ADDR   0x27

void I2C_Init(void);
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_Print(const char *str);
void LCD_PrintInt(int val);

#endif