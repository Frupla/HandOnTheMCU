#include "buttons.h"

#define white   0x00FFFFFF
#define black   0x00000000
#define green   0x0042f462
#define red     0x001010ed
#define yellow  0x0007ebef
#define blue    0x00ef072d


struct buttonTag initButton(int X1,int Y1,int X2,int Y2) // x1, y1, x2, y2
{
  struct buttonTag thisButton;
  
  thisButton.x1 = X1;
  thisButton.y1 = Y1;
  thisButton.x2 = X2;
  thisButton.y2 = Y2;
  thisButton.pressed = 0;
  thisButton.state = 0;
  thisButton.wasTheScreenTouchedInTheLastCycle = 0;
  thisButton.screenUpdate = 1;
  return thisButton;
}


void drawButton(struct buttonTag *button)
{
  if(button->screenUpdate){
    GLCD_SetColors(black,black);
    GLCD_SetWindow(button->x1-2, button->y1-2, button->x2+2, button->y2+2);
    GLCD_TextSetPos(0,0);
    GLCD_print("\f");
    if(button->state){
    GLCD_SetColors(black,green);
    }else{
    GLCD_SetColors(black,red);
    }
    GLCD_SetWindow(button->x1, button->y1, button->x2, button->y2);
    GLCD_TextSetPos(0,0);
    GLCD_print("\f");
    GLCD_SetColors(black,white);
    button->screenUpdate = 0;
  }
}

int within(struct buttonTag *button, int x, int y)
{
  if((button->x1 < x && x <= button->x2) && (button->y1 < y && y <= button->y2)){
    return 1;
  }else{
    return 0;
  }
}


int checkPressed(struct buttonTag *button)
{
  ToushRes_t XY_Touch;
  if(!TouchGet(&XY_Touch)){
    button->wasTheScreenTouchedInTheLastCycle = false;
  }
  
  
  button->pressed = (within(button, XY_Touch.X, XY_Touch.Y) && !(button->wasTheScreenTouchedInTheLastCycle)); 

  if(button->pressed){
    button->wasTheScreenTouchedInTheLastCycle = true;
  }
  
  if(!(button->state) && button->pressed){
    button->state = 1;
    button->screenUpdate = 1;
    return 1;
  }
  if(button->state && button->pressed){
    button->state = 0;
    button->screenUpdate = 1;
    return 0;
  }
  
  return button->state;
}
