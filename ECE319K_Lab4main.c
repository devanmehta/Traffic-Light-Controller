/* ECE319K_Lab4main.c
 * Traffic light FSM
 * ECE319H students must use pointers for next state
 * ECE319K students can use indices or pointers for next state
 * Put your names here or look silly
  */

#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/UART.h"
#include "../inc/Timer.h"
#include "../inc/Dump.h"  // student's Lab 3
#include <stdio.h>
#include <string.h>
// put your EID in the next line
const char EID1[] = "DJM5987"; //  ;replace abc123 with your EID
// Hint implement Traffic_Out before creating the struct, make struct match your Traffic_Out

// initialize all 6 LED outputs and 3 switch inputs
// assumes LaunchPad_Init resets and powers A and B
void Traffic_Init(void){ // assumes LaunchPad_Init resets and powers A and B
 // SOUTH LEDs (PB 0,1,2)
  IOMUX->SECCFG.PINCM[PB0INDEX] = 0x81;   // PB0 GPIO
  IOMUX->SECCFG.PINCM[PB1INDEX] = 0x81;   // PB1 GPIO
  IOMUX->SECCFG.PINCM[PB2INDEX] = 0x81;   // PB2 GPIO

  //WEST LEDs (PB6,7,8)
  IOMUX->SECCFG.PINCM[PB6INDEX] = 0x81;  
  IOMUX->SECCFG.PINCM[PB7INDEX] = 0x81;   
  IOMUX->SECCFG.PINCM[PB8INDEX] = 0x81;   
  

  // Make them outputs
  GPIOB->DOE31_0 |= (1<<0)|(1<<1)|(1<<2)|
                    (1<<6)|(1<<7)|(1<<8);

  // SWITCHES (PB15,16,17)
  IOMUX->SECCFG.PINCM[PB15INDEX] = 0x00040081;  
  IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00040081;  
  IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00040081;   
  
}
/* Activate LEDs
* Inputs: west is 3-bit value to three east/west LEDs
*         south is 3-bit value to three north/south LEDs
*         walk is 3-bit value to 3-color positive logic LED on PB22,PB26,PB27
* Output: none
* - west =1 sets west green
* - west =2 sets west yellow
* - west =4 sets west red
* - south =1 sets south green
* - south =2 sets south yellow
* - south =4 sets south red
* - walk=0 to turn off LED
* - walk bit 22 sets blue color
* - walk bit 26 sets red color
* - walk bit 27 sets green color
* Feel free to change this. But, if you change the way it works, change the test programs too
* Be friendly*/
  void Traffic_Out(uint32_t west, uint32_t south, uint32_t walk){

  // Clear all traffic LEDs
  GPIOB->DOUT31_0 &= ~((1<<0)|(1<<1)|(1<<2)|   //south LEDS
                      (1<<6)|(1<<7)|(1<<8));    //West leds
      
  // Clear walk LEDs
  GPIOB->DOUT31_0 &= ~((1<<22)|(1<<26)|(1<<27));

  // Set South LEDs (PB0-2)
  GPIOB->DOUT31_0 |= (south << 0); // turns on the correct led based n input of 001(g) 010(y) 100(r)

  // Set West LEDs (PB6-8)
  GPIOB->DOUT31_0 |= (west << 6); //same rules except g,y,r just 6+ the original pb0,1,2 (pb6,7,8)

  // Set Walk LEDs
  GPIOB->DOUT31_0 |= walk; //orrs with its bitmask
}

/* Read sensors
 * Input: none
 * Output: sensor values
 * - bit 0 is west car sensor
 * - bit 1 is south car sensor
 * - bit 2 is walk people sensor
* Feel free to change this. But, if you change the way it works, change the test programs too
 */
uint32_t Traffic_In(void){
  uint32_t input;
  uint32_t west, south, walk;

  // Read all of Port B inputs
  input = GPIOB->DIN31_0;

  // Extract each sensor bit
  west  = (input >> 15) & 0x01; //Moves PB15 down to bit0 and masks everything else off.
  south = (input >> 16) & 0x01; //Moves PB16 down to bit0 and masks everything else off.
  walk  = (input >> 17) & 0x01; //Moves PB17 down to bit0 and masks everything else off.

  // Pack into 3-bit number: walk south west
  return (walk << 2) | (south << 1) | west; //returns it as a 3 bit val example 001
}




// Output Macros
#define G 1
#define Y 2
#define R 4
#define W_WHITE ((1<<22)|(1<<26)|(1<<27))
#define W_RED   (1<<26)
#define W_OFF   0

// Define the State structure using indices
struct State {
  uint32_t outSouth;
  uint32_t outWest;
  uint32_t outWalk;
  uint32_t delay;        // Wait time in units of 10ms
  uint32_t Next[8];      // Array of indices to the next states
};
typedef const struct State State_t;

// FSM Array definition (Mapping exactly to your state transition table)
State_t fsm[13] = {
  // 0: goS        Next8: (W) (S) (S,W) (Walk) (Walk, W) (Walk, S) (All 3)  //the 8 possible inputs 
  {G, R, W_RED, 100, { 0, 1, 0, 1, 1, 1, 1, 1 }},     //what states it will go to based off the input
  // 1: waitS
  {Y, R, W_RED, 50,  { 2, 2, 2, 2, 2, 2, 2, 2 }},
  // 2: allRedS
  {R, R, W_RED, 20
  ,  { 0, 10,0, 10,3, 3, 3, 3 }},
  // 3: goWalk
  {R, R, W_WHITE,100,{ 3, 4, 4, 4, 3, 4, 4, 4 }},
  // 4: flash1 (Red)
  {R, R, W_RED, 20,  { 5, 5, 5, 5, 5, 5, 5, 5 }},
  // 5: flash2 (Off)
  {R, R, W_OFF, 20,  { 6, 6, 6, 6, 6, 6, 6, 6 }},
  // 6: flash3 (Red)
  {R, R, W_RED, 20,  { 7, 7, 7, 7, 7, 7, 7, 7 }},
  // 7: flash4 (Off)
  {R, R, W_OFF, 20,  { 8, 8, 8, 8, 8, 8, 8, 8 }},
  // 8: flash5 (Red)
  {R, R, W_RED, 20,  { 9, 9, 9, 9, 9, 9, 9, 9 }},
  // 9: allRedWk
  {R, R, W_RED, 20,  { 3, 10,0, 10,3, 10,0, 10 }},
  // 10: goW
  {R, G, W_RED, 100, { 10,10,11,11,11,11,11,11 }},
  // 11: waitW
  {R, Y, W_RED, 50,  { 12,12,12,12,12,12,12,12 }},
  // 12: allRedW
  {R, R, W_RED, 20,  { 10,10,0, 0, 3, 3, 0, 0 }}
};

// Global variable to track the current state index
uint32_t S;











// use main1 to determine Lab4 assignment
void Lab4Grader(int mode);
void Grader_Init(void);
int main1(void){ // main1
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Lab4Grader(0); // print assignment, no grading
  while(1){
  }
}



// use main2 to debug LED outputs
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_Out  function
int main2(void){ // main2
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
  if((GPIOB->DOE31_0 & 0x20)==0){
    UART_OutString("access to GPIOB->DOE31_0 should be friendly.\n\r");
  }
  UART_Init();
  UART_OutString("Lab 4, Spring 2026, Step 1. Debug LEDs\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  while(1){
  
    
    Traffic_Out(0, 1, 0); //South Green
    Debug_Dump(1);             // Record test
    Clock_Delay(80000000);     // Wait  1 seciond

   
    Traffic_Out(0, 2, 0);  //South Yellow 
    Debug_Dump(2);
    Clock_Delay(80000000); 

    Traffic_Out(0, 4, 0); //  South Red 
    Debug_Dump(4);
    Clock_Delay(80000000);

  
    Traffic_Out(1, 0, 0);  //West Green 
    Debug_Dump(8);         
    Clock_Delay(80000000);

  
    Traffic_Out(2, 0, 0); // West Yellow
    Debug_Dump(16);
    Clock_Delay(80000000);

  
    Traffic_Out(4, 0, 0); // West Red 
    Debug_Dump(32);
    Clock_Delay(80000000);

    
    Traffic_Out(0, 0, (1<<22)|(1<<26)|(1<<27)); //Walk white
    Debug_Dump(64);
    Clock_Delay(80000000);

    
    Traffic_Out(0, 0, (1<<26)); //Walk red
    Debug_Dump(128);
    Clock_Delay(80000000);
    
    if((GPIOB->DOUT31_0&0x20) == 0){
      UART_OutString("DOUT not friendly\n\r");
    }
  }
}



// use main3 to debug the three input switches
// at this point in ECE319K you need to be writing your own test functions
// modify this program so it tests your Traffic_In  function
int main3(void){ // main3
  uint32_t last=0,now;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Traffic_Init(); // your Lab 4 initialization
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2026, Step 2. Debug switches\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
  while(1){
    now = Traffic_In(); // Your Lab4 input
    if(now != last){ // change
      UART_OutString("Switch= 0x"); UART_OutUHex(now); UART_OutString("\n\r");
      Debug_Dump(now);
    }
    last = now;
    Clock_Delay(800000); // 10ms, to debounce switch
  }
}



// use main4 to debug using your dump
// proving your machine cycles through all states
int main(void){// main4
uint32_t input;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
 // set initial state
  Debug_Init();   // Lab 3 debugging
  UART_Init();
  __enable_irq(); // UART uses interrupts
  UART_OutString("Lab 4, Spring 2026, Step 3. Debug FSM cycle\n\r");
  UART_OutString("EID1= "); UART_OutString((char*)EID1); UART_OutString("\n\r");
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits

  while(1){
    
   // 1) output depending on state using Traffic_Out
      Traffic_Out(fsm[S].outWest, fsm[S].outSouth, fsm[S].outWalk);

    // Array lookup table: index 0 maps to 0 (Off), index 1 maps to 2 (Red), index 3 maps to 7 (White)
      uint32_t WalkTranslation[4] = {0, 2, 0, 7};

      // Translate the massive Walk bitmask into 0, 2, or 7 using array lookup
      uint32_t walkDump = WalkTranslation[fsm[S].outWalk >> 26]; //shifts the walk bitmask over by 26 since the walk led is on pb27,pb26,pb22.
                                                                 //pb26 is red and moving it over 26 will give value of 1 so walktranslation[1] = 2 (010 aka red)
                                                                 //pb27,26,22 is white and moving it over 26 will drop pb22 but keep pb27,26 as 011 which is 3 so walktranslation[3] = 7 (111 aka white)
                                                                 
      // Pack the 4 values into a single 32-bit variable to put into debug dump
      uint32_t dumpValue = (S << 24) |                      // State number in bits 31-24
                  (fsm[S].outWest << 16) |         // West output in bits 23-16
                  (fsm[S].outSouth << 8) |         // South output in bits 15-8
                  (walkDump);                      // Translated Walk output in bits 7-0

      // Call your Debug_Dump logging your state number and output
      Debug_Dump(dumpValue);

      // 2) Wait depending on state
      SysTick_Wait10ms(fsm[S].delay);
      
      // 3) Hard code this so input always shows all switches pressed 
      input = 7; 
      
      // 4) Next depends on state and input
      S = fsm[S].Next[input];
  }
}



// use main5 to grade
int main5(void){// main5
uint32_t input;
  Clock_Init80MHz(0);
  LaunchPad_Init();
  Grader_Init(); // execute this line before your code
  LaunchPad_LED1off();
  Traffic_Init(); // your Lab 4 initialization
// initialize your FSM
  SysTick_Init();   // Initialize SysTick for software waits
  // initialize your FSM
  Lab4Grader(1); // activate UART, grader and interrupts
  while(1){
      // 1) output depending on state using Traffic_Out
      // call your Debug_Dump logging your state number and output
      // 2) wait depending on state
      // 3) input from switches
      // 4) next depends on state and input

      // 1) Output depending on state using Traffic_Out
      Traffic_Out(fsm[S].outWest, fsm[S].outSouth, fsm[S].outWalk);
      
      // 2) Wait depending on state
      SysTick_Wait10ms(fsm[S].delay);
      
      // 3) Input from switches
      input = Traffic_In();
      
      // 4) Next depends on state and input
      S = fsm[S].Next[input];


      
  }
}

