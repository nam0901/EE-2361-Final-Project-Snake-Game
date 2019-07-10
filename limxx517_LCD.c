//This is a library of functions for the LCD display
#include "xc.h"
#include "limxx517_LCD.h"
#include "limxx517_Lab5_asmLib.h"
#include <string.h>
#include <p24Fxxxx.h>
#include <p24FJ64GA002.h>

void delay(unsigned int delay_in_ms)
{
    while(delay_in_ms-- > 0)
    {
        wait_1ms();
    }
}

void lcd_wait(void)
{
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
}

void lcd_cmd(char command)
{
    I2C2CONbits.SEN = 1;      //Initiate start condition
    lcd_wait();
    I2C2TRN = 0b01111100;     //Transmitting slave address and write bit
    lcd_wait();
    I2C2TRN = 0b00000000;     //Transmitting control byte
    lcd_wait();
    I2C2TRN = command;        //transmitting the data byte
    lcd_wait();
    I2C2CONbits.PEN = 1;      //Initiate stop condition
    lcd_wait();
    wait_30us();
}

void lcd_init(unsigned char contrast)
{
    delay(50);           //50 ms delay (waiting for VDD to stabilize)
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00111001); // function set, extended instruction mode
    lcd_cmd(0b00010100); // interval osc frequency
    lcd_cmd(0b01110000 | (contrast & 0b00001111)); // contrast C3-C0
    lcd_cmd(0b01010100 | ((contrast >> 4) & 0b00000011)); // C5-C4, Ion, Bon
    lcd_cmd(0b01101100); // follower control
    delay(200);          // 200 ms delay (for power stable))
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00001100); // Display On
    lcd_cmd(0b00000001); // Clear Display
    delay(2);            //Delay 2 ms (Initialization end))
}

void lcd_setCursor(char row, char column)
{
    lcd_cmd(0x80 | (0x40*row + column));
}

void lcd_clearDisplay(void)
{
    lcd_cmd(0b00000001);
}

void lcd_printChar(char myChar)
{
    I2C2CONbits.SEN = 1;      //Initiate start condition
    lcd_wait();
    I2C2TRN = 0b01111100;     //Transmitting slave address and write bit
    lcd_wait();
    I2C2TRN = 0b01000000;     //Transmitting control byte
    lcd_wait();
    I2C2TRN = myChar;        //transmitting the data byte
    lcd_wait();
    I2C2CONbits.PEN = 1;      //Initiate stop condition
    lcd_wait();
}

void lcd_printStr(const char* str, unsigned int delay_per_char) //same as lcd_printStr()....just added delay effect
{
    I2C2CONbits.SEN = 1;      //Initiate start condition
    lcd_wait();
    I2C2TRN = 0b01111100;     //Transmitting slave address and write bit
    lcd_wait();
    
    unsigned int index = 0;
    while(index < strlen(str))
    {
        if(index == 8) //if we have printed all chars on Row 0...
        {
            I2C2TRN = 0b10000000;     //setting command to write to DDRAM (Co = 1, Ro = 0))
            lcd_wait();
            I2C2TRN = 0b11000000;     //setting cursor to Row 1, Column 0
            lcd_wait();
        }
        if(index == strlen(str) - 1) //last char in string
        {
            I2C2TRN = 0b01000000;     //Co = 0, RS = 1
        }
        else //not last char in String
        {
            I2C2TRN = 0b11000000;
        }
        lcd_wait();
        I2C2TRN = str[index++];   //transmitting the data byte
        lcd_wait();
        delay(delay_per_char);
    } 
    
    I2C2CONbits.PEN = 1;      //Initiate stop condition
    lcd_wait();
}

void lcd_setContrast(char contrast) //Range (0-63)
{
    lcd_cmd(0b00111001);   //IS on extended mode
    lcd_cmd(0b01110000 | (contrast & 0b00001111)); // contrast C3-C0
    lcd_cmd(0b01010100 | ((contrast >> 4) & 0b00000011)); // C5-C4, Ion, Bon
    lcd_cmd(0b00111000);   //IS on normal mode
}

void lcd_blink(char* str, unsigned int blink_rate) //simulate blinking effect
{
    lcd_printStr(str, 0);
    delay(1000/(2*blink_rate));
    lcd_clearDisplay();
    delay(1000/(2*blink_rate));
}

void lcd_setDisplayFont(unsigned short height)//single or double height
{
    if(height == 1)//single
    {
        lcd_cmd(0b00110100);
    }
    else //double
    {
        lcd_cmd(0b00111000);
    }
}

void lcd_shiftDisplay(char dir)
{
    if(dir == 'r')//shift to the right
    {
        lcd_cmd(0b00011100);
    }
    else //shift to the left
    {
        lcd_cmd(0b00011000);
    }
}

void lcd_reset()
{
    lcd_clearDisplay();
    lcd_setCursor(0,0);
}