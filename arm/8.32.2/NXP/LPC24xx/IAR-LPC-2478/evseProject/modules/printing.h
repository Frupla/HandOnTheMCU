#ifndef PRINTING_H_
#define PRINTING_H_

#include <stdio.h>
#include "drv_glcd.h"

extern int yPositionForPrinting;
extern int xPositionForPrinting;


/*************************************************************************
 * Function Name: changeX
 * Parameters: int
 *
 * Return: none
 *
 * Description: Moves x
 *
 *************************************************************************/
void changeX(int);
/*************************************************************************
 * Function Name: changeY
 * Parameters: int
 *
 * Return: none
 *
 * Description: Moves y
 *
 *************************************************************************/
void changeY(int);
/*************************************************************************
 * Function Name: getX
 * Parameters: none
 *
 * Return: int32U
 *
 * Description: gives the position of x
 *
 *************************************************************************/
int getX();
/*************************************************************************
 * Function Name: getY
 * Parameters: none
 *
 * Return: int
 *
 * Description: gives the position of y
 *
 *************************************************************************/
int getY();
/*************************************************************************
 * Function Name: newLine
 * Parameters: none
 *
 * Return: none
 *
 * Description: Moves y 24 pixels down
 *
 *************************************************************************/
void newLine();
/*************************************************************************
 * Function Name: resetCursor
 * Parameters: none
 *
 * Return: none
 *
 * Description: Moves x and y to 0,0
 *
 *************************************************************************/
void resetCursor();
/*************************************************************************
 * Function Name: printFloat
 * Parameters: float
 *
 * Return: none
 *
 * Description: Prints a float with three decimal precision
 *
 *************************************************************************/
void printFloat(float);
/*************************************************************************
 * Function Name: printString
 * Parameters: char*
 *
 * Return: int
 *
 * Description: Prints a String and the return how many pixels it took takes to write it TODO
 *
 *************************************************************************/
int printString(char*);
/*************************************************************************
 * Function Name: printInt
 * Parameters: int
 *
 * Return: none
 *
 * Description: Prints an integer 
 *
 *************************************************************************/
int printInt(int);

#endif