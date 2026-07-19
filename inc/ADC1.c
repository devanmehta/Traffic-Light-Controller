/* ADC1.c
 * Students put your names here
 * Modified: put the date here
 * 12-bit ADC input on ADC1 channel 5, PB18
 */
#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"


void ADCinit(void){
ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) Reset
  ADC1->ULLMEM.GPRCM.PWREN  = 0x26000001; // 2) Power ON
  Clock_Delay(24);                         // 3) Wait
  ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // 4) ULPCLK
  ADC1->ULLMEM.CLKFREQ       = 7;          // 5) 40-48 MHz
  ADC1->ULLMEM.CTL0          = 0x03010000; // 6) Divide by 8
  ADC1->ULLMEM.CTL1          = 0x00000000; // 7) Software trigger
  ADC1->ULLMEM.CTL2          = 0x00000000; // 8) MEMRES[0]
  ADC1->ULLMEM.MEMCTL[0]     = 6;          // 9) Default channel 6 = PB19
  ADC1->ULLMEM.SCOMP0        = 0;          // 10) 8 sample clocks
  ADC1->ULLMEM.CPU_INT.IMASK = 0;          // 11) No interrupt
}


// ------------------------------------------------------------
// ADC_Player1 - PB19, ADC1 channel 6 (Right Paddle)
// ------------------------------------------------------------
uint32_t ADC_Player1(void){
  ADC1->ULLMEM.CTL0          = 0x03010000; // disable
  ADC1->ULLMEM.MEMCTL[0]     = 6;          // channel 6 = PB19
  ADC1->ULLMEM.CTL0          = 0x03010001; // enable
  ADC1->ULLMEM.CTL1          = 0x00000100; // start
  uint32_t volatile delay    = ADC1->ULLMEM.STATUS;
  while((ADC1->ULLMEM.STATUS & 0x01) == 0x01){}
  return ADC1->ULLMEM.MEMRES[0];
}

// ------------------------------------------------------------
// ADC_Player2 - PB18, ADC1 channel 5 (Left Paddle)
// ------------------------------------------------------------
uint32_t ADC_Player2(void){
  ADC1->ULLMEM.CTL0          = 0x03010000; // disable first (ENC=0)
  ADC1->ULLMEM.MEMCTL[0]     = 5;          // switch to channel 5 = PB18
  ADC1->ULLMEM.CTL0          = 0x03010001; // enable
  ADC1->ULLMEM.CTL1          = 0x00000100; // start
  uint32_t volatile delay    = ADC1->ULLMEM.STATUS;
  while((ADC1->ULLMEM.STATUS & 0x01) == 0x01){}
  uint32_t result            = ADC1->ULLMEM.MEMRES[0];
  ADC1->ULLMEM.CTL0          = 0x03010000; // disable again
  ADC1->ULLMEM.MEMCTL[0]     = 6;          // restore channel 6 = PB19
  return result;
}


// Maps 0-4095 → paddle top-edge Y pixel (0-87)
// Landscape LCD: 128px tall, 10px boundary, 30px paddle
// Max top edge = 128 - 30 - 10 = 87 ← this is correct 
uint32_t Convert(uint32_t input){
  
  return (input * 87) / 4095;


  
}

// do not use this function for the final lab
// it is added just to show you how SLOW floating point in on a Cortex M0+
//float FloatConvert(uint32_t input){
//  return 0.00048828125*input -0.0001812345;
//}

