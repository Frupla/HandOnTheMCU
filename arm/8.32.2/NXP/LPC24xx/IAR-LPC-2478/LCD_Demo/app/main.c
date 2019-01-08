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
#include "Cursor.h"
#include "smb380_drv.h"

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

#define TIMER0_TICK_PER_SEC   1


extern FontType_t Terminal_6_8_6;
extern FontType_t Terminal_9_12_6;
extern FontType_t Terminal_18_24_12;

#define LCD_VRAM_BASE_ADDR ((Int32U)&SDRAM_BASE_ADDR)

Int32U timetick = 0;

/*************************************************************************
 * Function Name: Timer0IntrHandler
 * Parameters: none
 *
 * Return: none
 *
 * Description: Timer 0 interrupt handler
 *
 *************************************************************************/
void Timer0IntrHandler (void)
{
  timetick++;
  // Toggle USB Link LED
  USB_D_LINK_LED_FIO ^= USB_D_LINK_LED_MASK | USB_H_LINK_LED_MASK;
  // clear interrupt
  T0IR_bit.MR0INT = 1;
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
typedef Int32U ram_unit;

  GLCD_Ctrl (FALSE);

  // Init GPIO
  GpioInit();
#ifndef SDRAM_DEBUG
  // MAM init
  MAMCR_bit.MODECTRL = 0;
  MAMTIM_bit.CYCLES  = 3;   // FCLK > 40 MHz
  MAMCR_bit.MODECTRL = 2;   // MAM functions fully enabled
  // Init clock
  InitClock();
  // SDRAM Init
  SDRAM_Init();
#endif // SDRAM_DEBUG
  // Init VIC
  VIC_Init();
  // GLCD init
  GLCD_Init (redScreenPic.pPicStream, NULL);
  
  GLCD_SetFont(&Terminal_18_24_12,0x0000FF,0x000cd4ff);
  GLCD_SetWindow(95,10,255,33);
  GLCD_TextSetPos(0,0);
  GLCD_print("\fHello world");

  GLCD_SetWindow(55,195,268,218);
  GLCD_TextSetPos(0,0);
  GLCD_print("\f 1 2 3");

  
  
  // Init USB Link  LED
  USB_D_LINK_LED_FDIR = USB_D_LINK_LED_MASK | USB_H_LINK_LED_MASK;
  USB_D_LINK_LED_FSET = USB_D_LINK_LED_MASK;// | USB_H_LINK_LED_MASK;
  
 
  // Enable TIM0 clocks
  PCONP_bit.PCTIM0 = 1; // enable clock

  // Init Time0
  T0TCR_bit.CE = 0;     // counting  disable
  T0TCR_bit.CR = 1;     // set reset
  T0TCR_bit.CR = 0;     // release reset
  T0CTCR_bit.CTM = 0;   // Timer Mode: every rising PCLK edge
  T0MCR_bit.MR0I = 1;   // Enable Interrupt on MR0
  T0MCR_bit.MR0R = 1;   // Enable reset on MR0
  T0MCR_bit.MR0S = 0;   // Disable stop on MR0
  // set timer 0 period
  T0PR = 0;
  T0MR0 = SYS_GetFpclk(TIMER0_PCLK_OFFSET)/(TIMER0_TICK_PER_SEC);
  // init timer 0 interrupt
  T0IR_bit.MR0INT = 1;  // clear pending interrupt
  VIC_SetVectoredIRQ(Timer0IntrHandler,0,VIC_TIMER0);
  VICINTENABLE |= 1UL << VIC_TIMER0;
  T0TCR_bit.CE = 1;     // counting Enable
  __enable_interrupt();
  GLCD_Ctrl (TRUE);
  
  
  while(1){

  }
}
