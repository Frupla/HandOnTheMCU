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
 *    2. Date        : 7, January 2019
 *       Author      : Irene Danvy, Frederik Ettrup Larsen and Jacob Frederiksen
 *       Description : Modified to complete the course "Hands-on microcrontroller programming"
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
#include "redScreen.h"
#include "blackScreen.h"
#include "printing.h"
#include "buttons.h"
#include <math.h>
   
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

#define P19_MASK (1UL<<19)
#define P11_MASK (1UL<<11)

#define white   0x00FFFFFF
#define black   0x00000000
#define green   0x0042f462
#define red     0x001010ed
#define yellow  0x0007ebef
#define blue    0x00ef072d

Int32U timetick = 0;
Int32U x2 = 0;
float x2_real = 0;
Int32U x3 = 0;
float x3_real =0;
Int32U y2_old = 0;
Int32U y3_old = 0;
Int32U t_old = 0;
Int32U x2_old = 0;
Int32U x3_old = 0;
int i2 = 0;
int i3 = 0;
#define N_O_PERIODS 16
#define channel3 false
#define channel2 true
float crosstick2 = 0;
float crosstick2_old = 0;
float crosstick3 = 0;
float crosstick3_old = 0;
float alpha =0;
float T2 = 0;
float T3 = 0;
//float f = 0;
Boolean VrefInTheLastCycle = false;
Boolean tickCrossingZero2 = false;
Boolean tickCrossingZero3 = false;
Boolean waitingForCross2 = true;
Boolean waitingForCross3 = true;
Boolean timeToPrint = true;

//Doing current, voltage and power calculations
float currentSum = 0;
float lastCompletedCurrentSum = 0;
Int32U lastCompletedCurrentSumSamples = 0;
Boolean newCurrentSum = true;
Int32U currentSamples = 0;

float voltageSum = 0;
float lastCompletedVoltageSum = 0;
Int32U lastCompletedVoltageSumSamples = 0;
Boolean newVoltageSum = true;
Int32U voltageSamples = 0;

/*************************************************************************
 * Function Name: lowPass
 * Parameters: x
 *
 * Return: y
 *
 * Description: low passes the signal
 *
 *************************************************************************/
Int32U lowPass(Int32U x, Boolean channel){
  Int32U y = 0;
  if (channel){
    y   = (Int32U)(alpha*x + (1 - alpha)*y2_old);
    y2_old = y;
  } else{
    y   = (Int32U)(alpha*x + (1 - alpha)*y3_old);
    y3_old = y;
  }
  y = (y*1.033); //attempts correcting for approx -3dB attenuation, more or less magic number
  /*
  if (y>1023){
    y = 1023;
  }
  */
  return (Int32U)y;
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
 //FIO0PIN |= P11_MASK;
//  DACR_bit.VALUE = 0x03FF;
  timetick++;
    // Toggle USB Link LED
  
  if(timetick > 5000){
    USB_D_LINK_LED_FIO ^= USB_D_LINK_LED_MASK | USB_H_LINK_LED_MASK;
    tickCrossingZero2 = true;
    tickCrossingZero3 = true;
    timeToPrint = true;
    timetick = 0;
  }
   
  // Read voltage ADC
  if(ADDR2_bit.DONE){
    x2_old = x2;
    x2 = lowPass(ADDR2_bit.RESULT,channel2);

    //x2_real = (((float)3.3*x2)/1023)-1.65; 
    //RMS of 240V gives peak to peak = 2*sqrt(2)*240V = 678.8225V
    //Halfway: 339.41125
    x2_real = (((float)678.8225*x2)/1023)-339.41125; 
    voltageSum += x2_real*x2_real;
    voltageSamples++;
    //DACR_bit.VALUE = x2;
  }
  
  //Read current ADC
  if(ADDR3_bit.DONE){
    x3_old = x3;
    //x3 = ADDR3_bit.RESULT;
    //peak to peak = 1A*2*sqrt(2)= 2.8
    x3 = lowPass(ADDR3_bit.RESULT,channel3);
    x3 = x3-4;//correct for leakage current - ok, nvm, varies wildly with heat so impossible
    x3_real = (((float)2.8284*x3)/1023)-1.4142; //
    currentSum += x3_real*x3_real;
    currentSamples++;
  }


  
  /* 
  if(x <= 0x200 && x >= 0x1FE && !VrefInTheLastCycle){   // If x is Vref/2
    VrefInTheLastCycle = true;
    
    if(crossingZero){
      f = 1 /(float)(timetick+5000 - t_old);
      crossingZero = false;
    }else{
      f = 1/(float)(timetick - t_old);
    }
    t_old = timetick;
  }else{
    VrefInTheLastCycle = false;
  }
  */
  
  
  // Channel 2, aka voltage measurements
  if (waitingForCross2 && x2 >= 512){
    if (i2 >= N_O_PERIODS){
      crosstick2 = timetick - (float)x2/((float)x2-(float)x2_old);
      if (tickCrossingZero2){
        T2 = crosstick2+5000 - crosstick2_old;
        tickCrossingZero2 = false;
      } else{
        T2 = crosstick2 - crosstick2_old;
      }
      crosstick2_old = crosstick2;
      lastCompletedCurrentSum = currentSum;
      lastCompletedCurrentSumSamples = currentSamples;
      currentSum = 0;
      currentSamples = 0;
      lastCompletedVoltageSum = voltageSum;
      lastCompletedVoltageSumSamples = voltageSamples;
      voltageSum = 0;
      voltageSamples = 0;
      newCurrentSum = true;
      i2 = 0;
    }
    waitingForCross2 = false;
    i2++;
  } else if(!waitingForCross2 && x2 <= 512){
    waitingForCross2 = true;
  }
 
  /* This was a misunderstandning, don't do this'
  //Channel3, aka current measurements
  if (waitingForCross3 && x3 >= 512){ //310
    if (i3 <= N_O_PERIODS) {
      crosstick3 = timetick - (float)x3/((float)x3-(float)x3_old);
      if (tickCrossingZero3){
        T3 = crosstick3+5000 - crosstick3_old;
        tickCrossingZero3 = false;
      } else{
        T3 = crosstick3 - crosstick3_old;
      }
      crosstick3_old = crosstick3;
      i3=0;
    }
    waitingForCross3 = false;
    i3++;
  } else if(!waitingForCross3 && x3 <= 512){
    waitingForCross3 = true;
  }
*/
  //FIO0PIN &= ~P11_MASK;
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
 // PCLKSEL0_bit.PCLK_ADC = 0x1; //Enable ADC clock
  AD0CR_bit.CLKDIV = 8; //18MHz/(8+1)= 2MHz <= 4.5 MHz?7+1)= 4MHz<=4.5 MHz
  AD0CR_bit.BURST = 1; //0=ADC is set to operate in software controlled mode, 1= continue mode
 // PINSEL1_bit.P0_23 = 0x1; //AD0[0]
  //PINMODE1_bit.P0_23 = 0x2;
  PINSEL1_bit.P0_25 = 0x1; //AD0[2]
  PINMODE1_bit.P0_25 = 0x2;
  PINSEL1_bit.P0_26 = 0x1; //AD0[3]
  PINMODE1_bit.P0_26 = 0x2;
  /*ADINTEN_bit.ADGINTEN = 0; //When 0, only the individual A/D channels enabled by ADINTEN 7:0 will generate interrupts.
  ADINTEN_bit.ADINTEN0 = 1; //Enable interrupt
  ADINTEN_bit.ADINTEN1 = 1;
  ADINTEN_bit.ADINTEN2 = 1;
  ADINTEN_bit.ADINTEN3 = 1;*/
  AD0CR_bit.START = 0;
  //VIC_SetVectoredIRQ(ADC_Intr_Handler,TS_INTR_PRIORITY,VIC_AD0);
  //VICINTENABLE |= 1UL << VIC_AD0;
  AD0CR_bit.SEL |= 0x0F; // Channel 0, 1, 2 and 3 enabled: 1111 = 15 = F
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
Int32U cursor_x = (C_GLCD_H_SIZE - CURSOR_H_SIZE)/2, cursor_y = (C_GLCD_V_SIZE - CURSOR_V_SIZE)/2;
ToushRes_t XY_Touch;


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
 GLCD_Init (blackScreenPic.pPicStream, NULL);
 GLCD_SetFont(&Terminal_9_12_6,black,white);


  GLCD_Cursor_Dis(0);

  GLCD_Copy_Cursor ((Int32U *)Cursor, 0, sizeof(Cursor)/sizeof(Int32U));

  GLCD_Cursor_Cfg(CRSR_FRAME_SYNC | CRSR_PIX_32);

  GLCD_Move_Cursor(cursor_x, cursor_y);

  GLCD_Cursor_En(0);

  

  
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

  // Init ADC
  ADC_Init();
  // Init touch screen
  TouchScrInit(); 
  
  __enable_interrupt();
  GLCD_Ctrl (TRUE);
  GLCD_SetFont(&Terminal_18_24_12,0x00ffffff,0x0000000);
  

/*
  //initialize DAC - commented out as we need the pin for current sampling
  /*
  PINSEL1_bit.P0_26=2; //sets pin function to AOUT
  DACR_bit.BIAS=1; //set BIAS mode 1
  PCLKSEL0_bit.PCLK_DAC=1; //enable clock signal
  DACR_bit.VALUE = 0X3FF;
  */
   // Filter calculations
   Int32U fc = 50; //Value of cut off frequency
   float RC = (1/(2*3.1415*fc));
   alpha = (1/(float)TIMER1_TICK_PER_SEC)/(RC+(1/(float)TIMER1_TICK_PER_SEC));
   float F2 = 0;
   float F3 = 0;
   //float current_test = 0;
   
   //Current, Voltage and Power constants
   float currentRMS = 0;
   float voltageRMS = 0;
   
   struct buttonTag toggleAutoBulb = initButton(10,150,60,200);
   struct buttonTag toggleBulb = initButton(70,150,120,200);
   struct buttonTag toggleAutoPlug = initButton(130,150,180,200);
   struct buttonTag togglePlug = initButton(190,150,240,200);
   
   drawButton(&toggleAutoBulb);
   drawButton(&toggleBulb);
   drawButton(&toggleAutoPlug);
   drawButton(&togglePlug); 
   
   FIO0DIR = P19_MASK | P11_MASK; // Setting pin 19 to be an output
   printString("\fLive data:");
   int l1 = printString("\fFrequency (V): \0");
   int l2 = printString("\fI(RMS): \0");
   int l3 = printString("\fV(RMS): \0");
   int l4 = printString("\fx2: \0");
   int l5 = printString("\fx3: \0");
   int l6 = printString("\fBulb: \0");
     
   
   while(1){     
   // TimerIntr_Handler(); 
   resetCursor();
   newLine();
   
    // Here we handle all the printing that goes of twice a second
   if(timeToPrint){
     
     resetCursor();
     newLine();
     //Current calculation
     if (newCurrentSum){
       currentRMS = sqrt(2*lastCompletedCurrentSum/(lastCompletedCurrentSumSamples));
       //currentRMS = currentRMS*sqrt((3.3*3.3)/((float)(1023*1023))); //yet to be found
       voltageRMS = sqrt(2*lastCompletedVoltageSum/lastCompletedVoltageSumSamples);
       //voltageRMS = voltageRMS*sqrt((3.3*3.3)/((float)(1023*1023))); //converting from digi - (0-3.3V)
       newCurrentSum = false;
       
     }
       
       
     // Here we handle all the printing that goes of twice a second
     if(timeToPrint){
        
        //Calculating and printing voltage frequency
        F2 = TIMER1_TICK_PER_SEC*N_O_PERIODS/T2;
        //Calculating and printing voltage frequency
        F3 = TIMER1_TICK_PER_SEC*N_O_PERIODS/T3;
        
        changeX(l1);        
        printFloatAndUnit(F2,"Hz");
        changeX(l2);
        printFloatAndUnit(currentRMS,"A");
        changeX(l3);
        printFloatAndUnit(voltageRMS,"V");
        changeX(l4);
        printFloat(x2_real);
        changeX(l5);
        printInt(x3);
        changeX(l6);
        if(FIO0PIN & P19_MASK){
	    printString("\fON");
        }else{
	    printString("\fOFF");
        }
        
        changeX(0);
        
        timeToPrint = false;
   }else{
     newLine();
     newLine();
     newLine();
   }
    
  if(TouchGet(&XY_Touch)) // Cursor mover
  {
     cursor_x = XY_Touch.X;
     cursor_y = XY_Touch.Y;
     GLCD_Move_Cursor(cursor_x, cursor_y);
  }
  
  
   drawButton(&toggleAutoBulb);
   drawButton(&toggleBulb);
   drawButton(&toggleAutoPlug);
   drawButton(&togglePlug); 
   
   if(checkPressed(&toggleAutoBulb)){
     if(checkPressed(&toggleBulb)){
       FIO0PIN |= P19_MASK;
    }else{
       FIO0PIN &= ~P19_MASK;
     }
   }else if(F2 < 49 && F2 > 51){
       FIO0PIN &= ~P19_MASK;
     }else{
       FIO0PIN |= P19_MASK;
    }
   
   if(checkPressed(&toggleAutoPlug)){
     if(checkPressed(&togglePlug)){
       FIO0PIN |= P11_MASK;
    }else{
       FIO0PIN &= ~P11_MASK;
    }
   }else if(F3 < 49 && F3 > 51){
       FIO0PIN &= ~P11_MASK;
    }else{
       FIO0PIN |= P11_MASK;    
   }
   }
  }
}
