#include "xc.h"
#include "finalProject_gameFunctions.h"
#include "lab4_assembly.h"
#include <stdlib.h>
#include <stdint.h> /* Includes uint16_t definition */
#include <stdbool.h> /* Includes true/false definition */
#include <string.h>
#include <p24Fxxxx.h>
#include <p24FJ64GA002.h>

//private macros
#define SNAKEHEAD currentSnakePosition[0]
#define SNAKETAIL currentSnakePosition[currentSnakeLength - 1]
#define MAX_GROWTHSTATE 8
#define MIN_ROW -1 //due to weird bug in DDRAM data
#define MIN_COLUMN 0
#define MAX_ROW 6 //due to weird bug in DDRAM data
#define MAX_COLUMN 7

//private typedef declarations
typedef enum{
    noInput,
    inputUp,
    inputDown,
    inputLeft,
    inputRight
}UserInput;

/*global variable definitions*/

//snake handling
Direction currentSnakeDirection;
unsigned int currentSnakeLength;
Coordinate currentSnakePosition[MAX_SNAKELENGTH];
unsigned int currentGrowthState ;
UserInput prevInput;

// food handling
Coordinate currentFoodPosition;
Coordinate bonusFoodPosition;
Coordinate bombPosition[2]; // 3 dots
unsigned int numOfMeals;
unsigned int currentScore;
unsigned int highScore = 0;
volatile unsigned int T1rollovers = 0;
volatile unsigned int T2rollovers = 0;

bool isEqual(Coordinate firstCoordinate, Coordinate secondCoordinate) //compare Coordinates for equality
{
    return (firstCoordinate.row == secondCoordinate.row) && (firstCoordinate.column == secondCoordinate.column);
}

bool collidesWithSnake(Coordinate food) //checks if food placement overlaps with the snake
{
    for(unsigned int index = 0; index < currentSnakeLength; index++)
    {
        if(isEqual(food, currentSnakePosition[index]))
        {
            return true;
        }
    }
    return false;
}
/*functions*/
//bonus Food
bool collideswithFood(Coordinate input){
    
    return ((currentFoodPosition.column == input.column) && (currentFoodPosition.row == input.row)) ;
}

void generateBonusFood() //generate bonus Food - 3s
{
  Coordinate temp ;
  do{
temp.row = rand() % 8 -1 ;
temp.column = rand() % 8;
  }while(collidesWithSnake(temp) && collideswithFood(temp));
  
  bonusFoodPosition.row = temp.row;
  bonusFoodPosition.column = temp.column;
}

bool eatBonus(){
    return isEqual(SNAKEHEAD, bonusFoodPosition);
}

void setupTimer(){
    //setting up Timer 1 - 20ms
    T1CON = 0x0010; //pre-scale 1:8
    TMR1 = 0x0000;
    PR1 = 40000;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;//Enable Timer 1
  
}


void __attribute__((__interrupt__,__auto_psv__))_T1Interrupt(void)
{
    T1rollovers++;
    IFS0bits.T1IF = 0;
}
void resetBonusFood(){
    TMR1 = 0;
    T1rollovers = 0;
    generateBonusFood();
}

void updateBonusDisplay(char *matrixDisplay){
    //display only 3s
   
            if(bonusFoodPosition.row == -1) //exception due to weird bug in DDRAM data
    {
        matrixDisplay[2*bonusFoodPosition.column] |= (1 << 7);
    }
    else
    {
        matrixDisplay[2*bonusFoodPosition.column] |= (1 << bonusFoodPosition.row);
    }

        if(T1rollovers > 150) //3 seconds display
            {
                TMR1 = 0;
                T1rollovers = 0;
                generateBonusFood();
            }
           
}


//helper functions
void stringToStruct(Coordinate *toAssign, const char** toCopy, unsigned int length) //helper function for snake_init(). Takes an array of strings
{                                                                                   //and initializes the Coordinate array based on it
    for(unsigned index = 0; index < length; index++)
    {
        toAssign[index].row = (int)(toCopy[index][1] - '0');
        toAssign[index].column = (int)(toCopy[index][3] - '0');
    }
}



bool isUpwards()
{
    return SNAKETAIL.row == currentSnakePosition[currentSnakeLength - 2].row - 1;
}

bool isDownwards()
{
    return SNAKETAIL.row == currentSnakePosition[currentSnakeLength - 2].row + 1;
}

bool isLeftwards()
{
    return SNAKETAIL.column == currentSnakePosition[currentSnakeLength - 2].column - 1;
}

bool isRightwards()
{
    return SNAKETAIL.column == currentSnakePosition[currentSnakeLength - 2].column + 1;
}

//user input handling
unsigned int readKeyPadRAW() //check for heypress on the matrix
{
    //checking keypress on all rows, one iteration per row
    const unsigned int LATBoutputs[4] = {0xEFFF, 0xDFFF, 0xBFFF, 0x7FFF};
    
    for(unsigned int index = 0; index < 4; index++)
    {
        LATB = LATBoutputs[index];
        for(unsigned int index = 0; index < 5; index++); //give a short delay
        
        switch(PORTA)
        {
            case 0x0017:
                return 1 + 4*index;
                
            case 0x001B:
                return 2 + 4*index;
                
            case 0x001D:
                return 3 + 4*index;
                
            case 0x001E:
                return 4 + 4*index;
        }
    }
    return 0; //no input
}
UserInput filterUserInput(unsigned int keypress) //processes user input from the keypad matrix and filters off unmeaningful input and keyspams
{
    switch(keypress)
    {
        case 2:
            if(prevInput == inputUp || prevInput == inputDown)
            {
                return noInput;
            }
            else
            {
                return inputUp;
            }
            
        case 5:
            if(prevInput == inputLeft || prevInput == inputRight)
            {
                return noInput;
            }
            else
            {
                return inputLeft;
            }
            
        case 7:
            if(prevInput == inputLeft || prevInput == inputRight)
            {
                return noInput;
            }
            else
            {
                return inputRight;
            }
            
        case 10:
            if(prevInput == inputUp || prevInput == inputDown)
            {
                return noInput;
            }
            else
            {
                return inputDown;
            }
            
        default:
            return noInput;
    }   
}

void pollUserInput(unsigned int delay_in_ms) //poll for user input in between frames. Updates the snake's current direction
{                                             //if meaningful input is received. Frame delay is delay_in_ms
    UserInput newUserInput;
    for(unsigned int index = 0; index < delay_in_ms; index++)
    {
        if((newUserInput = filterUserInput(readKeyPadRAW()))) //if user input is meaningful...
        {
            currentSnakeDirection = newUserInput;
        }
        wait_1ms();
    }
    prevInput = currentSnakeDirection; //assign prevInput to the latest user input(before this function exits))
}

//snake handling
void snakeReset(Direction initialSnakeDirection, unsigned int initialSnakeLength, const char** initialSnakePosition)
{   //resets the snake's properties and game state
    currentSnakeDirection = initialSnakeDirection;
    prevInput = initialSnakeDirection;
    currentSnakeLength = initialSnakeLength;
    stringToStruct(currentSnakePosition, initialSnakePosition, initialSnakeLength);
    currentGrowthState = 0;
    numOfMeals = 0;
    currentScore = 0;
}

void updateSnakePosition() //update the snake's position on the grid based on the snake's current direction
{
    for(unsigned int index = currentSnakeLength - 1; index > 0; index--)
    {
        currentSnakePosition[index].row = currentSnakePosition[index - 1].row; //basic array shift
        currentSnakePosition[index].column = currentSnakePosition[index - 1].column;
        //tail vanishes, body follows head
    }
    if(currentSnakeDirection == up)
    {
        ++SNAKEHEAD.row; //snake's head moves upwards
    }
    else if(currentSnakeDirection == down)
    {
        --SNAKEHEAD.row; //snake's head moves downwards
    }
    else if(currentSnakeDirection == left)
    {
        --SNAKEHEAD.column; //snake's head move towards the left
    }
    else if(currentSnakeDirection == right)
    {
        ++SNAKEHEAD.column; //snake's head move towards the right
    }
}

void updateSnakeDisplay(char* matrixDisplay) //set the new values of the display data according to the current snake position
{
    for(unsigned int index = 0;  index < currentSnakeLength; index++)
    {
        if(currentSnakePosition[index].row == -1) //exception due to weird bug in DDRAM data
        {
            matrixDisplay[2*currentSnakePosition[index].column] |= (1 << 7);
        }
        else
        {
            matrixDisplay[2*currentSnakePosition[index].column] |= (1 << currentSnakePosition[index].row);
        }
    }
}

void increaseNumOfMeals()
{
    ++numOfMeals;
}

bool timeToGrow()
{
    return !(numOfMeals%3) && (currentGrowthState < MAX_GROWTHSTATE);
}

void snakeGrowth()
{
    if(currentSnakeLength < MAX_SNAKELENGTH) //if snake still has room to become longer...
    {
         if(isUpwards())
        {
            currentSnakePosition[currentSnakeLength].row = SNAKETAIL.row - 1;
            currentSnakePosition[currentSnakeLength].column = SNAKETAIL.column;
        }
        else if(isDownwards())
        {
            currentSnakePosition[currentSnakeLength].row = SNAKETAIL.row + 1;
            currentSnakePosition[currentSnakeLength].column = SNAKETAIL.column;
        }
        else if(isLeftwards())
        {
            currentSnakePosition[currentSnakeLength].row = SNAKETAIL.row;
            currentSnakePosition[currentSnakeLength].column = SNAKETAIL.column + 1;
        }
        else if(isRightwards())
        {
            currentSnakePosition[currentSnakeLength].row = SNAKETAIL.row;
            currentSnakePosition[currentSnakeLength].column = SNAKETAIL.column - 1;
        }
    
        ++currentSnakeLength;
    }
   
    ++currentGrowthState;
}

unsigned int delayBetweenFrames()
{
    return 500 - 50*currentGrowthState;
}

//food handling
bool eatSuccess() //checks if snake has eaten the food
{
    return isEqual(SNAKEHEAD, currentFoodPosition);
}

void generateNewFood() //generate and assign a new unique food position
{
    Coordinate potentialFood;

    do{
        potentialFood.row = rand()%8 - 1; //exception due to weird bug in DDRAM data
        potentialFood.column = rand()%8;
    }while(collidesWithSnake(potentialFood)); //check for overlap of food with snake

    currentFoodPosition.row = potentialFood.row; //food does not overlap, update currentFoodPosition
    currentFoodPosition.column = potentialFood.column;
}

void updateFoodDisplay(char* matrixDisplay) //updates the display data
{
    if(currentFoodPosition.row == -1) //exception due to weird bug in DDRAM data
    {
        matrixDisplay[2*currentFoodPosition.column] |= (1 << 7);
    }
    else
    {
        matrixDisplay[2*currentFoodPosition.column] |= (1 << currentFoodPosition.row);
    }
}

//game state handlers
void updateNormal()
{
    switch(currentGrowthState)
    {
        case 0:
            currentScore += 3;
            break;
            
        case 1:
            currentScore += 5;
            break;
            
        case 2:
            currentScore += 7;
            break;
            
        case 3:
            currentScore += 10;
            break;
            
        case 4:
            currentScore += 13;
            break;
            
        case 5:
            currentScore += 15;
            break;
            
        case 6:
            currentScore += 18;
            break;
            
        case 7:
            currentScore += 20;
            break;
            
        default: //case 8
            currentScore += 25;
    }
}

void updateBonus()
{
    switch(currentGrowthState)
    {
        case 0:
            currentScore += 4;
            break;
        case 1:
            currentScore += 5;
            break;
        case 2:
            currentScore += 6;
            break;
        case 3:
            currentScore += 8;
            break;
        case  4:
            currentScore += 10;
            break;
        case 5:
            currentScore += 15;
            break;
        case 6:
            currentScore += 20;
            break;
        case 7:
            currentScore += 25;
            break;
        default:
            currentScore += 30;
    }           
}

bool isHighScore()
{
    return currentScore > highScore;
}

void updateHighScore()
{
    highScore = currentScore;
}

bool outOfBounds()
{
    return (SNAKEHEAD.row < MIN_ROW) || (SNAKEHEAD.row > MAX_ROW)
            ||(SNAKEHEAD.column < MIN_COLUMN) || (SNAKEHEAD.column > MAX_COLUMN);
}

bool selfCollision() //check if snake collides with itself
{
    for(unsigned int index = 1; index < currentSnakeLength; index++) //comparing the head to other parts of the snake
    {
        if(isEqual(SNAKEHEAD,currentSnakePosition[index]))
        {
            return true; //there is a collision
        }
    }
    return false; //loop fallen through, no equality spotted
}

bool gameOver()
{
    return outOfBounds() || selfCollision();
}
