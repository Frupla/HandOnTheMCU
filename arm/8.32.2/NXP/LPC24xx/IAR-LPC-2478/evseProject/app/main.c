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
 *    1. Date        : 12, August 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *  This example project shows how to use the IAR Embedded Workbench for ARM
 * to develop code for the IAR LPC2478-SK board. It shows basic use of I/O,
 * timers, interrupts, LCD controllers and LCD touch screen.
 *
 *  A cursor is shown and moves when the screen is touched.
 *
 * Jumpers:
 *  EXT/JLINK  - depending of power source
 *  ISP_E      - unfilled
 *  RST_E      - unfilled
 *  BDS_E      - unfilled
 *  C/SC       - SC
 *
 * Note:
 *  After power-up the controller gets it's clock from internal RC oscillator that
 * is unstable and may fail with J-Link auto detect, therefore adaptive clocking
 * should always be used. The adaptive clock can be select from menu:
 *  Project->Options..., section Debugger->J-Link/J-Trace  JTAG Speed - Adaptive.
 *
 * The LCD shares pins with Trace port. If ETM is enabled the LCD will not work.
 *
 *    $Revision: 28 $
 **************************************************************************/
#include <includes.h>

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

#define LCD_VRAM_BASE_ADDR ((Int32U)&SDRAM_BASE_ADDR)

extern Int32U SDRAM_BASE_ADDR;
extern FontType_t Terminal_6_8_6;
extern FontType_t Terminal_9_12_6;
extern FontType_t Terminal_18_24_12;


#define TIMER1_TICK_PER_SEC   10000

Int32U timetick = 0;
Int32U y_old = 0;
Int32U t_old = 0;
float f = 0;
Boolean VrefInTheLastCycle = false;
Boolean crossingZero = false;

/*************************************************************************
 * Function Name: lowPass
 * Parameters: x
 *
 * Return: y
 *
 * Description: low passes the signal
 *
 *************************************************************************/
Int32U lowPass(Int32U x, Int32U fc){
  float RC = (1/(2*3.1415*fc));
  
  float alpha = (1/(float)TIMER1_TICK_PER_SEC)/(RC+(1/(float)TIMER1_TICK_PER_SEC));
  
  Int32U y   = (Int32U)(alpha*x + (1 - alpha)*y_old);
  
  
  y_old = y;
  
  return y;
}
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
//  DACR_bit.VALUE = 0x03FF;
  timetick++;
  // Toggle USB Link LED
  if(timetick - t_old > 5000){
    USB_D_LINK_LED_FIO ^= USB_D_LINK_LED_MASK | USB_H_LINK_LED_MASK;
    crossingZero = true;
    timetick = 0;
  }
  
  Int32U x = 0;
  
  if(ADDR2_bit.DONE){
    x = lowPass(ADDR2_bit.RESULT,50);
    DACR_bit.VALUE = x;
  }
  

  

  if(x <= 0x200 && x >= 0x1FE && !VrefInTheLastCycle){   // If x is Vref/2
    VrefInTheLastCycle = true;
    
    if(crossingZero){
      f = 1/(float)(timetick+5000 - t_old);
      crossingZero = false;
    }else{
      f = 1/(float)(timetick - t_old);
    }
    t_old = timetick;
  }else{
    VrefInTheLastCycle = false;
  }

  
  // clear interrupt
  T1IR_bit.MR1INT = 1;
  VICADDRESS = 0;
}


/*************************************************************************
 * Function Name: ADC_init
 * Parameters: none
 *
 * Return: none
 *
 * Description: Initialises adc
 *
 *************************************************************************/
void ADC_Init (void){ 
  AD0CR_bit.PDN = 0;
  PCONP_bit.PCAD = 1; //PCAD A/D converter (ADC) power/clock control bit. Note: Clear the PDN bit in
                      // the AD0CR before clearing this bit, and set this bit before setting PDN.
  PCLKSEL0_bit.PCLK_ADC = 0x1; //Enable ADC clock
  AD0CR_bit.CLKDIV = 5; //18MHz/(5+1)= 3MHz<=4.5 MHz?7+1)= 4MHz<=4.5 MHz
  AD0CR_bit.BURST = 1; //0=ADC is set to operate in software controlled mode, 1= continue mode
  PINSEL1_bit.P0_25 = 0x1; //AD0[2]
  PINMODE1_bit.P0_25 = 0x2;
  //PINSEL1_bit.P0_26 = 0x1; //AD0[3]
  //PINMODE1_bit.P0_26 = 0x2;
  ADINTEN_bit.ADGINTEN = 0; //When 0, only the individual A/D channels enabled by ADINTEN 7:0 will generate interrupts.
  ADINTEN_bit.ADINTEN0 = 1; //Enable interrupt
  ADINTEN_bit.ADINTEN1 = 1;
  ADINTEN_bit.ADINTEN2 = 1;
  ADINTEN_bit.ADINTEN3 = 1;
//  AD0CR_bit.START = 1;
//  VIC_SetVectoredIRQ (TIMER1IntrHandler,2,VIC_TIMER1);
//  VICINTENABLE |= 1UL << VIC_TIMER1;
  AD0CR_bit.SEL = 0x04; // Channel 0, 1, 2 and 3 enabled. 1111
  AD0CR_bit.PDN = 1; //The A/D Converter is operational
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
//Int32U cursor_x = (C_GLCD_H_SIZE - CURSOR_H_SIZE)/2, cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE)/2;
//ToushRes_t XY_Touch;
//Boolean Touch = FALSE;

//  GLCD_Ctrl (FALSE);
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
/*  GLCD_Init (IarLogoPic.pPicStream, NULL);

  GLCD_Cursor_Dis(0);

  GLCD_Copy_Cursor ((Int32U *)Cursor, 0, sizeof(Cursor)/sizeof(Int32U));

  GLCD_Cursor_Cfg(CRSR_FRAME_SYNC | CRSR_PIX_32);

  GLCD_Move_Cursor(cursor_x, cursor_y);

  GLCD_Cursor_En(0);

  // Init touch screen
  TouchScrInit();
*/
  
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
 
  ADC_Init();
  
  __enable_interrupt();
 /* GLCD_Ctrl (TRUE);


  GLCD_SetFont(&Terminal_18_24_12,0x0000FF,0x000cd4ff);
  GLCD_SetWindow(95,10,255,33);
  GLCD_TextSetPos(0,0);
  GLCD_print("\fIAR Systems");

  GLCD_SetWindow(55,195,268,218);
  GLCD_TextSetPos(0,0);
  GLCD_print("\fTouch screen demo");
*/
  
  //initialize DAC
  PINSEL1_bit.P0_26=2; //sets pin function to AOUT
  DACR_bit.BIAS=1; //set BIAS mode 1
  PCLKSEL0_bit.PCLK_DAC=1; //enable clock signal
  DACR_bit.VALUE = 0X3FF;

  while(1){
 
  }
}
