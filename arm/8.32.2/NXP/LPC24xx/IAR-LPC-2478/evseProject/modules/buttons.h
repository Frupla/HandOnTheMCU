#ifndef __BUTTONS_H
#define __BUTTONS_H


#include "drv_glcd.h"
#include "drv_touch_scr.h"

typedef struct buttonTag{
  int x1,x2,y1,y2;
  int pressed;
  int state;
  int wasTheScreenTouchedInTheLastCycle;
  int screenUpdate;
}button;

struct buttonTag initButton(int,int,int,int); // x1, y1, x2, y2

void drawButton(struct buttonTag *button);

int within(struct buttonTag *button, int, int);

int checkPressed(struct buttonTag *button);

#endif