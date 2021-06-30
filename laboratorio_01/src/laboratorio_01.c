#include <stdint.h>
#include <stdbool.h>
// includes from driverlib library
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"


void main(void){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enables GPIO F (LED D4 = PF0)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // Waits for initialization
  
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // LED D4 as output
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0); // LED D4 off
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
  
  int timer = 0;   // To allow the LED to switch state on a given cycle
  int LED_D4 = 0;  // State of the LED which will be switchen from "on" to "off"
  
  while(1){
    if (timer<24000000)   //  Reference line
      timer ++;
    else{
      timer = 0;
      LED_D4 ^= GPIO_PIN_0;
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, LED_D4); // LED D4 on/off
    }
  } // while
} // main

/*
  Switching the optimization of the C compiler has a direct influence on the
  performance of each cycle (which is related to the different compiler selection
  of instructions to be executed). For instance, with the "Medium" setting it is
  possible to observe the LED blinking at a rate close to once per second,
  however, switching the setting to "High" makes it blink faster.
  
  Regarding the clock frequency, it is natural that the higher its value, the
  faster the microcontroller will be able to perform instructions. Nevertheless,
  the LED will only blink faster if the value of the timer (set in the Reference
  line of the main code above) does not change accordingly. If it does — proportionally
  to how the clock frequency is changed — then the LED will maintain its blinking
  frequency (give or take slight differences due to the compiler optimization).
*/