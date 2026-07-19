/*!
 * @defgroup FIFO
 * @brief Lab 8H First in first out queue, C++
 * @{*/
/**
 * @file      FIFO2.h
 * @brief     Provide functions for a first in first out queue
 * @details   ECE319H Lab 8H.  
 * @author    change this to your names or look very silly
 * @date     change this to the last modification date or look very silly
 */

#ifndef __FIFO2_H__
#define __FIFO2_H__
#include <stdint.h>
/**
 * \brief FIFOSIZE the size of the Lab 8 FIFO, which can hold 0 to FIFOSIZE-1 elements.
 * The size must be a power of 2.
 */
#define FIFOSIZE 32  // maximum storage is FIFOSIZE-1 elements 
/**
 * ECE319K Lab 8H FIFO 
 * @brief C++ FIFO queue
 */
class Queue{ 
private:
  char Buf[FIFOSIZE];
  int PutI; // index to an empty place, next place to put
  int GetI; // index to oldest data, next to get

public:
  Queue();            // initialize queue
  bool IsEmpty(void); // true if empty
  bool IsFull(void);  // true if full
  bool Put(char x);   // enter data into queue
  bool Get(char *pt); // remove data from queue
  void Print(void);   // display element of queue on LCD
};


#endif //  __FIFO2_H__
/** @}*/
