#ifndef FINALPROJECT_LEDRIVER_H
#define FINALPROJECT_LEDRIVER_H

//constants
#define NUM_OF_ADDRESSES 16

//variables to be exported
extern char matrixDisplayData[NUM_OF_ADDRESSES];

//functions to be exported
void LEDriver_setup();
void LEDriver_writeDisplay();
void clearDisplayData();
void LEDriver_displayOn();
void LEDriver_displayBlink();
void LEDriver_displayOff();

#endif
