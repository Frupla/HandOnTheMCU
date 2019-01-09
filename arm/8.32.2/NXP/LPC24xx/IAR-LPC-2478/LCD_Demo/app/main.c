/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : main.c
 *    Description : Main module
 *
 *    History :
 *    1. Date        : 4, August 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    2. Date        : 9, September 2009
 *       Author      : Frederick Hjärner
 *       Description : Updated the example description
 *
 * This example project shows how to use the IAR Embedded Workbench for ARM
 * to develop code for IAR-LPC-2478 board. It shows basic use of I/O,
 * timer, interrupt and LCD controllers.
 *
 * The IAR, NXP and Olimex logos appear on the LCD and the cursor
 * moves as the board moves(the acceleration sensor is used).
 *
 * Jumpers:
 *  EXT/JLINK  - depending of power source
 *  ISP_E      - unfilled
 *  RST_E      - unfilled
 *  BDS_E      - unfilled
 *  C/SC       - SC
 *
 * Note:
 *  After power-up the controller get clock from internal RC oscillator that
 * is unstable and may fail with J-Link auto detect, therefore adaptive clocking
 * should always be used. The adaptive clock can be select from menu:
 *  Project->Options..., section Debugger->J-Link/J-Trace  JTAG Speed - Adaptive.
 *
 * The LCD shares pins with Trace port. If ETM is enabled the LCD will not work.
 *
 *    $Revision: 28 $
 **************************************************************************/
#include <intrinsics.h>
#include <stdio.h>
#include "board.h"
#include "sys.h"
#include "sdram_64M_32bit_drv.h"
#include "drv_touch_scr.h"
#include "drv_glcd.h"
#include "Logo.h"
#include "redScreen.h"
#include "excuseMe.h"
#include "Cursor.h"
#include "smb380_drv.h"

#include <assert.h>
#include <nxp/iolpc2478.h>


#define NONPROT 0xFFFFFFFF
#define CRP1  	0x12345678
#define CRP2  	0x87654321
/*If CRP3 is selected, no future factory testing can be performed on the device*/
#define CRP3  	0x43218765

#ifndef SDRAM_DEBUG
#pragma segment=".crp"
#pragma location=".crp"
__root const unsigned crp = NONPROT;
#endif

#define TIMER1_TICK_PER_SEC   1000


extern FontType_t Terminal_6_8_6;
extern FontType_t Terminal_9_12_6;
extern FontType_t Terminal_18_24_12;

#define LCD_VRAM_BASE_ADDR ((Int32U)&SDRAM_BASE_ADDR)

Int32U timetick = 0;

/*************************************************************************
 * Function Name: TIMER1IntrHandler
 * Parameters: none
 *
 * Return: none
 *
 * Description: Timer 1 interrupt handler
 *
 *************************************************************************/
void TIMER1IntrHandler (void)
{
  timetick++;
  // Toggle USB Link LED
  if(timetick > 500){
    USB_D_LINK_LED_FIO ^= USB_D_LINK_LED_MASK | USB_H_LINK_LED_MASK;
    timetick = 0;
  }
/*  
  if(DACR_bit.VALUE >= 0x03FF){
    DACR_bit.VALUE = 0;
  }else{
    DACR_bit.VALUE += 50;
  }
*/

  
  // clear interrupt
  T1IR_bit.MR1INT = 1;
  VICADDRESS = 0;
}

/*************************************************************************
 * Function Name: main
 * Parameters: none
 *
 * Return: none
 *
 * Description: main
 *
 *************************************************************************/
int main(void)
{

ToushRes_t XY_touch;
Boolean touch = FALSE;
  
  GLCD_Ctrl (FALSE);
 
  GpioInit();
  
  MAMCR_bit.MODECTRL = 0;
  MAMTIM_bit.CYCLES  = 3;   // FCLK > 40 MHz
  MAMCR_bit.MODECTRL = 2;   // MAM functions fully enabled
  
  InitClock();
  // SDRAM Init'
  SDRAM_Init();

  // Init VIC
  VIC_Init();
  // GLCD init
  GLCD_Init (redScreenPic.pPicStream, NULL);
 
  // Init USB Link  LED
  USB_D_LINK_LED_FDIR = USB_D_LINK_LED_MASK | USB_H_LINK_LED_MASK;
  USB_D_LINK_LED_FSET = USB_D_LINK_LED_MASK;// | USB_H_LINK_LED_MASK;
  
 
  // Enable TIM1 clocks
  PCONP_bit.PCTIM1 = 1; // enable clock

  // Init Time1
  T1TCR_bit.CE = 0;     // counting  disable
  T1TCR_bit.CR = 1;     // set reset
  T1TCR_bit.CR = 0;     // release reset
  T1CTCR_bit.CTM = 0;   // Timer Mode: every rising PCLK edge
  T1MCR_bit.MR1I = 1;   // Enable Interrupt on MR0
  T1MCR_bit.MR1R = 1;   // Enable reset on MR0
  T1MCR_bit.MR1S = 0;   // Disable stop on MR0
  // set timer 1 period
  T1PR = 0;
  T1MR1 = SYS_GetFpclk(TIMER1_PCLK_OFFSET)/(TIMER1_TICK_PER_SEC);
  // init timer 1 interrupt
  T1IR_bit.MR1INT = 1;  // clear pending interrupt
  VIC_SetVectoredIRQ(TIMER1IntrHandler,0,VIC_TIMER1);
  VICINTENABLE |= 1UL << VIC_TIMER1;
  T1TCR_bit.CE = 1;     // counting Enable
  
  __enable_interrupt();
  GLCD_Ctrl (TRUE);

  // Init touch screen
  TouchScrInit();  
  
/*  PINSEL1_bit.P0_26=2; //sets pin function to AOUT
  DACR_bit.BIAS=1; //set BIAS mode 1
  PCLKSEL0_bit.PCLK_DAC=1; //enable clock signal
  DACR_bit.VALUE = 0X3FF;
*/
  
  GLCD_SetFont(&Terminal_18_24_12, 0x00ffffff, 0x000000);
  GLCD_SetWindow(55,195,268,218);
  GLCD_TextSetPos(0,0);
  GLCD_print("\f Waiting");

  
  while(1){
    char buffer [50];
    sprintf (buffer, "timetick: %d", timetick);
    GLCD_SetWindow(95,10,255,33);
    GLCD_TextSetPos(0,0);
    GLCD_print(buffer);
    
    if(TouchGet(&XY_touch)){
      GLCD_SetWindow(55,105,268,218);
      GLCD_TextSetPos(0,0);
      GLCD_print("\f SUCCEDES");
    }
    
    if(TouchGet(&XY_touch) && !touch){
       touch = true;
       GLCD_SetWindow(55,195,268,218);
       GLCD_TextSetPos(0,0);
       GLCD_print("\f SUCCEDES");
    }else if (!TouchGet(&XY_touch) && touch){
        touch = false;
        GLCD_SetWindow(55,195,268,218);
        GLCD_TextSetPos(0,0);
        GLCD_print("\f FAILS");
    }
  }
}
