#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS

osThreadId_t thread1_id, thread2_id, thread3_id, thread4_id;

// Struct to be passed as argument to the threads
struct selection
{
   int led;             // select which LED to use
   int act_period;      // select the activation period
};


void thread(void *arg){
  uint8_t state = 0;
  struct selection* select = arg;  // allow argument use inside the thread
    
  while(1){
    uint32_t tick;
    tick = osKernelGetTickCount();
    
    state ^= select->led;
    LEDWrite(select->led, state);
    
    osDelayUntil(tick + select->act_period);
  } // while
} // thread

void main(void){
  struct selection first = {LED1, 100};
  struct selection second = {LED2, 200};
  struct selection third = {LED3, 400};
  struct selection fourth = {LED4, 800};
  
  LEDInit(LED1 | LED2 | LED3 | LED4);
  
  osKernelInitialize();

  thread1_id = osThreadNew(thread, &first, NULL);
  thread2_id = osThreadNew(thread, &second, NULL);
  thread3_id = osThreadNew(thread, &third, NULL);
  thread4_id = osThreadNew(thread, &fourth, NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
