/* 
 * File:   limxx517_LCD.h
 * Author: Jan-Wei
 *
 * Created on October 19, 2017, 11:29 PM
 */

#ifndef LIMXX517_LCD_H
#define	LIMXX517_LCD_H

#ifdef	__cplusplus
extern "C" {
#endif
void delay(unsigned int);
void lcd_cmd(char);
void lcd_init(unsigned char);  
void lcd_setCursor(char, char);
void lcd_clearDisplay(void);
void lcd_printChar(char);
void lcd_printStr(const char*, unsigned int);
void lcd_setContrast(char);
void lcd_blink(char*, unsigned int);
void lcd_setDisplayFont(unsigned short);
void lcd_shiftDisplay(char);
void lcd_reset();
#ifdef	__cplusplus
}
#endif

#endif	/* LIMXX517_LCD_H */

