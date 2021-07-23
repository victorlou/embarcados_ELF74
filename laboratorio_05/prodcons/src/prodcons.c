#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverbuttons.h"

#define BUFFER_SIZE 8

osThreadId_t producer_id, consumer_id;
osSemaphoreId_t empty_id, full_id;
uint8_t buffer[BUFFER_SIZE];

volatile int event = 0;

// Activates the producer thread
void GPIOJ_Handler(void){
  ButtonIntClear(USW1);
  osEventFlagsNew = 1;
} // GPIOJ_Handler

void producer(void *arg){
  uint8_t index_i = 0, count = 0;
  
  while(1){
    if(event==1){
      event = 0;
      osSemaphoreAcquire(empty_id, osWaitForever); // is there space available?
      buffer[index_i] = count; // if so, add to buffer
      osSemaphoreRelease(full_id); // signal one less space
      
      index_i++; // increment buffer input index
      if(index_i >= BUFFER_SIZE)
        index_i = 0;
      
      count++;
      count &= 0x0F; // produce new information
      osDelay(500);
    }
  } // while
} // producer

void consumer(void *arg){
  uint8_t index_o = 0, state;
  
  while(1){
    osSemaphoreAcquire(full_id, osWaitForever); // is there space available?
    state = buffer[index_o]; // if so, remove from buffer
    osSemaphoreRelease(empty_id); // signal one more space
    
    index_o++; // increment buffer output index
    if(index_o >= BUFFER_SIZE)
      index_o = 0;
    
    LEDWrite(LED4 | LED3 | LED2 | LED1, state); // present consumed information
    osDelay(500);
  } // while
} // consumer

void main(void){
  SystemInit();
  LEDInit(LED4 | LED3 | LED2 | LED1);
  ButtonInit(USW1);
  ButtonIntEnable(USW1);

  osKernelInitialize();

  producer_id = osThreadNew(producer, NULL, NULL);
  consumer_id = osThreadNew(consumer, NULL, NULL);

  empty_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL); // available spaces = BUFFER_SIZE
  full_id = osSemaphoreNew(BUFFER_SIZE, 0, NULL); // occupied spaces = 0
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main