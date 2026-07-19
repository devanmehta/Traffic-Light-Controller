// FIFO1.c
// Runs on any microcontroller
// Provide functions that implement the Software FiFo Buffer
// Last Modified: July 19, 2025
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly
#include <stdint.h>


// Declare state variables for FiFo
//        size, buffer, put and get indexes

#define FIFOSIZE 16

static char FIFO[FIFOSIZE];
static uint32_t PutI;
static uint32_t GetI;
static uint32_t Size;


// *********** Fifo1_Init**********
// Initializes a software FIFO1 of a
// fixed size and sets up indexes for
// put and get operations
void Fifo1_Init(){
 // setting all conditions to 0
  PutI = 0;
  GetI = 0;
  Size = 0;
 
}

// *********** Fifo1_Put**********
// Adds an element to the FIFO1
// Input: data is character to be inserted
// Output: 1 for success, data properly saved
//         0 for failure, FIFO1 is full
uint32_t Fifo1_Put(char data){
  
  if(((PutI+1)%FIFOSIZE) == GetI){
        return 0;           // FIFO is full, fail
    }
    FIFO[PutI] = data;                  // store the byte
    PutI = (PutI + 1) % FIFOSIZE;      // advance and wrap
    Size++;
    return 1;               // success
}

// *********** Fifo1_Get**********
// Gets an element from the FIFO1
// Input: none
// Output: If the FIFO1 is empty return 0
//         If the FIFO1 has data, remove it, and return it
char Fifo1_Get(void){

    char data;
    if(GetI == PutI){
        return 0;           // FIFO is empty, return 0
    }
    data = FIFO[GetI];                  // read the byte
    GetI = (GetI + 1) % FIFOSIZE;      // advance and wrap
    Size--;
    return data;            // return the byte

}



