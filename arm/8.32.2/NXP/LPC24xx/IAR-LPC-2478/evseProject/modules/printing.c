#include "printing.h"


int yPositionForPrinting = 0;
int xPositionForPrinting = 0;


/*************************************************************************
 * Function Name: changeX
 * Parameters: int
 *
 * Return: none
 *
 * Description: Moves x
 *
 *************************************************************************/
void changeX(int x_new)
{
  xPositionForPrinting = x_new;
}
/*************************************************************************
 * Function Name: changeY
 * Parameters: int
 *
 * Return: none
 *
 * Description: Moves y
 *
 *************************************************************************/
void changeY(int y_new)
{
  yPositionForPrinting = y_new;
}
/*************************************************************************
 * Function Name: getX
 * Parameters: none
 *
 * Return: int32U
 *
 * Description: gives the position of x
 *
 *************************************************************************/
int getX()
{
  return xPositionForPrinting;
}
/*************************************************************************
 * Function Name: getY
 * Parameters: none
 *
 * Return: int
 *
 * Description: gives the position of y
 *
 *************************************************************************/
int getY()
{
  return yPositionForPrinting;
}
/*************************************************************************
 * Function Name: newLine
 * Parameters: none
 *
 * Return: none
 *
 * Description: Moves y 24 pixels down
 *
 *************************************************************************/
void newLine()
{
  yPositionForPrinting += 24;
}
/*************************************************************************
 * Function Name: resetCursor
 * Parameters: none
 *
 * Return: none
 *
 * Description: Moves x and y to 0,0
 *
 *************************************************************************/
void resetCursor()
{
  yPositionForPrinting = 0;
  xPositionForPrinting = 0;
}
/*************************************************************************
 * Function Name: printFloat
 * Parameters: float
 *
 * Return: none
 *
 * Description: Prints a float with three decimal precision
 *
 *************************************************************************/
void printFloat(float toPrint)
{
  if(yPositionForPrinting >= 240){
    yPositionForPrinting = 0;
  }
 
  char MyString [ 100 ]; // destination string
  int d,f1,f2,f3;
  d = (int) toPrint; // Decimal precision: 3 digits
  f1 = (int)(10*(toPrint-(float)d));
  f2 = (int)(100*(toPrint-(float)d)) - 10*f1;
  f3 = (int)(1000*(toPrint-(float)d)) - 10*f2 - 100*f1;
  sprintf ( MyString, "\f%d.%d%d%dHz", d, f1,f2,f3); 

  GLCD_SetWindow(xPositionForPrinting,yPositionForPrinting,319,25+yPositionForPrinting);
  GLCD_TextSetPos(0,0);
  GLCD_print(MyString);
  
  yPositionForPrinting += 24;
  
}
/*************************************************************************
 * Function Name: printString
 * Parameters: char*
 *
 * Return: int
 *
 * Description: Prints a String and the return how many pixels it took takes to write it TODO
 *
 *************************************************************************/
int printString(char* words){
  if(yPositionForPrinting >= 240){
    yPositionForPrinting = 0;
  }
  int length = 0;
  while(words[length+1] != '\0'){
    length++;
  }
  GLCD_SetWindow(xPositionForPrinting,yPositionForPrinting,319,25+yPositionForPrinting);
  GLCD_TextSetPos(0,0);
  GLCD_print(words);
   
  yPositionForPrinting += 24;
  return length;
}
/*************************************************************************
 * Function Name: printInt
 * Parameters: int
 *
 * Return: none
 *
 * Description: Prints an integer 
 *
 *************************************************************************/
void printInt(int number){
  if(yPositionForPrinting >= 240){
    yPositionForPrinting = 0;
  }
  GLCD_SetWindow(xPositionForPrinting,yPositionForPrinting,319,25+yPositionForPrinting);
  GLCD_TextSetPos(0,0);
  GLCD_print("\f%d",number);
   
  yPositionForPrinting += 24;
}
