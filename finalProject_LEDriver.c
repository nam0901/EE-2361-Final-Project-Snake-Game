/*This is a library of functions for HT16K33 LED Driver handling operations specifically for the snake game */

#include "xc.h"
#include <p24Fxxxx.h>
#include <p24FJ64GA002.h>
#include "finalProject_LEDriver.h"

//constants
#define NUM_OF_ADDRESSES 16

//variables
char matrixDisplayData[NUM_OF_ADDRESSES] = ""; //stores the matrix display data

//functions
void wait() //waiting for ACK from LED Driver
{
    while(!IFS3bits.MI2C2IF);
    IFS3bits.MI2C2IF = 0;
}

void LEDriver_cmd(char command) //sending command to LED Driver
{
    I2C2CONbits.SEN = 1; //initiate start condition
    wait();
    I2C2TRN = 0b11100000; //send slave address, write mode
    wait();
    I2C2TRN = command; //send command
    wait();
    I2C2CONbits.PEN = 1; //stop
    wait();
}

void LEDriver_setup()
{
    LEDriver_cmd(0b00100001); //system oscillator on (normal operation mode)
    LEDriver_cmd(0b10100000); //ROW as driver output
    LEDriver_cmd(0b10000001); //display on, blinking off
    LEDriver_cmd(0b11101111); //16/16 duty cycle
}

void LEDriver_writeDisplay() //utilizes matrixDisplayData to write the display data to the LED Driver
{
    I2C2CONbits.SEN = 1; //initiate start condition on the driver
    wait();
    I2C2TRN = 0b11100000; //send slave address
    wait();
    I2C2TRN = 0; //send reg address byte
    wait();
    for(unsigned int index = 0; index < 16; index++)
    {
        I2C2TRN = matrixDisplayData[index]; //sending display data
        wait();
    }
    I2C2CONbits.PEN = 1; //stop condition on the driver
    wait();
}

void clearDisplayData() //clears all display data
{
    for(unsigned int index = 0; index < 16; index++)
    {
        matrixDisplayData[index] = 0;
    }
}

void LEDriver_displayOn()
{
     LEDriver_cmd(0b10000001);
}

void LEDriver_displayBlink() //makes the display blink at 2Hz
{
    LEDriver_cmd(0b10000011);
}

void LEDriver_displayOff() //self explanatory
{
    clearDisplayData();
    LEDriver_writeDisplay();
}


