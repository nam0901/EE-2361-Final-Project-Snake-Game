#ifndef FINALPROJECT_GAMEFUNCTIONS_H
#define FINALPROJECT_GAMEFUNCTIONS_H

#include <stdbool.h>

//constants to be exported
#define MAX_SNAKELENGTH 8

//typedef declarations to be exported
typedef struct{
    int row;
    int column;
}Coordinate;

typedef enum{
    up = 1, //syncing with UserInput
    down,
    left,
    right
}Direction;

//variables to be exported
extern unsigned int currentScore;

//functions to be exported
unsigned int readKeyPadRAW();                                           //scan for keypress on the keypad matrix
void pollUserInput(unsigned int);                                       //polls the keypad matrix for user input in between frames
void snakeReset(Direction, unsigned int, const char**);                 //initializes/resets the snake's properties
void updateSnakePosition();                                             //updates the snake movement after every frame
void updateSnakeDisplay(char*);                                         //updates the state of the LEDs in the matrix after every frame
void increaseNumOfMeals();                                              //increases number of meals the snake has had
bool timeToGrow();                                                      //checks if its time for snake to increase a growth state
void snakeGrowth();                                                     //energy boost from food! increase the snake length and/or speed
unsigned int delayBetweenFrames();                                      //higher speed level, shorter delay between frames
bool eatSuccess();                                                      //checks if snake has eaten the food
void generateNewFood();                                                 //generates new food position
void updateFoodDisplay(char*);                                          //updates the state of the LEDs in the matrix after every frame
void updateScore();                                                     //updates user's current score
bool isHighScore();                                                     //tests if current score is high score
void updateHighScore();                                                 //current score is new high score, update it
bool gameOver();                                                        //checks whether the snake collides with itself or exits the matrix
//Bonus Food
bool collidesFood(Coordinate input);
void generateBonusFood();
bool eatBonus();
void setupTimer();
void updateBonusDisplay(char *matrixDisplay);
void resetBonusFood();
void updateBonus();

#endif
