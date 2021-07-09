#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "driverlib/systick.h"
#include "driverleds.h" // Projects/drivers

// FSM with one state for each binary state of the Grey Code
typedef enum {zero, one, three, two, six, seven, five, four} state_t;

// The state switch is periodically triggered by an exception using an Event flag
volatile uint8_t Event = 0;

// Sets the flag to switch to the following state
void SysTick_Handler(void){
  Event = 1;
} // SysTick_Handler

void main(void){
  static int Estado = zero; // initial FSM state
  
  // Initializes necessary LEDs and interruption period
  LEDInit(LED1);
  LEDInit(LED2);
  LEDInit(LED3);
  SysTickPeriodSet(12000000);
  SysTickIntEnable();
  SysTickEnable();

  while(1){
    if(Event){
      Event = 0;
      switch(Estado){
        case zero:
          LEDOff(LED1);
          LEDOff(LED2);
          LEDOff(LED3);
          Estado = one;
          break;
        case one:
          LEDOn(LED1);
          LEDOff(LED2);
          LEDOff(LED3);
          Estado = three;
          break;
        case three:
          LEDOn(LED1);
          LEDOn(LED2);
          LEDOff(LED3);
          Estado = two;
          break;
        case two:
          LEDOff(LED1);
          LEDOn(LED2);
          LEDOff(LED3);
          Estado = six;
          break;
        case six:
          LEDOff(LED1);
          LEDOn(LED2);
          LEDOn(LED3);
          Estado = seven;
          break;
        case seven:
          LEDOn(LED1);
          LEDOn(LED2);
          LEDOn(LED3);
          Estado = five;
          break;
        case five:
          LEDOn(LED1);
          LEDOff(LED2);
          LEDOn(LED3);
          Estado = four;
          break;
        case four:
          LEDOff(LED1);
          LEDOff(LED2);
          LEDOn(LED3);
          Estado = zero;
          break;
      } // switch
    } // if
  } // while
} // main
