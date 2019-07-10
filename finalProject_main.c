#include "xc.h"
#include "finalProject_LEDDriver.h"
#include "LCD.h"
#include "finalProject_gameFunctions.h"
#include "lab4_assembly.h"
#include <stdio.h>
#include <stdbool.h>
#include <p24Fxxxx.h>
#include <p24FJ64GA002.h>
#include <stdint.h> /* Includes uint16_t definition */
#include <stdbool.h> /* Includes true/false definition */

// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1 // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF // JTAG Port Enable (JTAG port is disabled)
// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME // Clock Switching and Monitor (Clock switching is enabled,
// Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

//global variables
char score[16] = "";
bool gameStart = 1;

//setup functions
void MCU_setup(void) //configure I2C2 and 4 x 4 keypad matrix
{
    CLKDIVbits.RCDIV = 0;  //internal instruction clock running at 16MHz

    //configuring 4 x 4 keypad matrix
    AD1PCFG = 0x9FFF;      //all pins digital
    TRISA = 0x001F;        //RA0, RA1, RA2, RA3 are inputs
    LATA = 0xFFFF;         //all pins initialized to high
    TRISB = 0x0003;        //RB0, RB1 inputs, rest are outputs
    LATB = 0xFFFF;         //all pins initialized to high
    CNPU1 = 0x000D;        //configuring pull-up resistors
    CNPU2 = 0x6000;        //...

    //configuring I2C module for communication with the LED driver
    I2C2CONbits.I2CEN = 0; //disabling I2C2 peripheral
    I2C2BRG = 157;         //setting up BRG (FSCL = 100kHz, FCY = 16MHz)
    I2C2CONbits.IPMIEN = 0;//operating as master, support mode disabled
    IFS3bits.MI2C2IF = 0;  //clearing I2C2 interrupt flag
    I2C2CONbits.I2CEN = 1; //enabling I2C2 peripheral
}

void gameReset() //initializing snake and food display
{
    snakeReset(right, 3, (const char*[MAX_SNAKELENGTH]){"(4,3)","(4,2)","(4,1)"});
    generateNewFood();
    generateBonusFood();
//    generateBomb();
    LEDriver_displayOn();
    lcd_printStr(" SCORE:   0000  ", 0);
}

//event handlers
void delay_in_ms(unsigned int delay) //self explanatory...
{
    for(unsigned int index = 0; index < delay; index++)
    {
        wait_1ms();
    }
}

void countdown(int numOfSeconds)
{
    char count[8] = "";
    for(int index = numOfSeconds; index >= 0; index--)
    {
        if(index)
        {
            sprintf(count, "   %d", index);
            lcd_printStr(count, 0);
        }
        else
        {
            lcd_printStr("   GO", 0);
        }
        
        delay_in_ms(1000);
        lcd_reset();
    }
}

void pollUserStart() //poll user input from keypad matrix to start game
{ 
    bool toggle = 0;
    while(1)
    {
        if(!toggle)
        {
            lcd_printStr(" Press  any key", 0);
        }
        else
        {
            lcd_printStr("   to     start", 0);
        }
        
        toggle = !toggle;
        
        for(unsigned int index = 0; index < 1000; index++)
        {
            if(readKeyPadRAW())
            {
                lcd_reset();
                return;
            }
            wait_1ms();
        }
        lcd_reset();
    }
}

void printUpdatedScore()
{
    sprintf(score," SCORE:   %04d  ", currentScore);
    lcd_reset();
    lcd_printStr(score, 0);
}

void gameOverEventHandler()
{
    LEDriver_displayBlink();
    lcd_reset();
    lcd_printStr("  GAME    OVER  ", 250);
    LEDriver_displayOff();
    lcd_reset();
    if(isHighScore())
    {
        updateHighScore();
        for(unsigned int index = 0; index < 5; index++) //just a random delay
        {
            lcd_blink("NEW HIGH SCORE!", 4);
        }
        lcd_reset();
    }
    for(unsigned int index = 0; index < 3; index++)
    {
       lcd_printStr(" FINAL   SCORE: ", 0);
       delay_in_ms(1000);
       lcd_reset();
       sprintf(score, "  %04d  ", currentScore);
       lcd_printStr(score, 0);
       delay_in_ms(1000);
       lcd_reset();
    }
}


int main()
{ 
    MCU_setup();
    LEDriver_setup();
    lcd_init(0b00100000);
    
    while(1)
    {   
        
        pollUserStart();
        countdown(3);
         gameReset();
         setupTimer();
         
         
        do //this loop represents a single frame cycle
        {
            clearDisplayData(); //clears the matrix display data
            updateSnakeDisplay(matrixDisplayData); //converts the snake's coordinates to writable DDRAM data
            updateFoodDisplay(matrixDisplayData); //converts the food's coordinate to writable DDRAM data
            updateBonusDisplay(matrixDisplayData); //converts the bonus coordinate to writable DDRAM data
            LEDriver_writeDisplay(); //writes the 8 x 8 matrix display
            
            if(eatSuccess())
            {
                increaseNumOfMeals();
                if(timeToGrow())
                {
                    snakeGrowth();
                }
                generateNewFood();
                updateNormal();
                printUpdatedScore();
            }
            
            else if(eatBonus())
           {
                
                
                updateBonus();
                printUpdatedScore();
                resetBonusFood();
            }
            
            
            pollUserInput(delayBetweenFrames()); //polls the keypad matrix for input for delayBetweenFrames() milliseconds. If there is an input, snake direction is updated.
            updateSnakePosition(); //updates the snake's coordinates on the grid depending on the snake's direction
            
            
        }while(!gameOver());
    
        //todo when game over
        gameOverEventHandler();
    
   
    }

    return 0;
}
