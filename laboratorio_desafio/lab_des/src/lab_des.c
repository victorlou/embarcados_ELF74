/* -------------------------------------------------------------------------- 
 * Observations:
 *      Attempting to use the following example as reference for PWM
 *    ti\TivaWare_C_Series-2.1.4.178\examples\peripherals\pwm\reload_interrupt.c
 *---------------------------------------------------------------------------*/

#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverbuttons.h"

#define QUEUE_SIZE 1

// Struct to be passed as argument to the threads
struct selection
{
   int led;             // select which LED to use
   int act_period;      // select the activation period
};

osThreadId_t tid_led1;                /* Thread id of thread: phase_a      */
osThreadId_t tid_led2;                /* Thread id of thread: phase_b      */
osThreadId_t tid_led3;                /* Thread id of thread: phase_c      */
osThreadId_t tid_led4;                /* Thread id of thread: phase_d      */
osThreadId_t tid_controladora;                 /* Thread id of thread: clock        */

osMutexId_t phases_mut_id;
osMessageQueueId_t mid_MsgQueue;

const osMutexAttr_t Phases_Mutex_attr = {
  "PhasesMutex",                            // human readable mutex name
  osMutexRecursive | osMutexPrioInherit,    // attr_bits
  NULL,                                     // memory for control block   
  0U                                        // size for control block
};


volatile uint8_t pwm_event = 0, led_event = 0;


// USAE BUTTON READ
// Identify which button was pressed to flag the correct event
void GPIOJ_Handler(void){
  if(!ButtonRead(USW1)){
    ButtonIntClear(USW1);
    led_event = 1;
  }
  else if (!ButtonRead(USW2)){
    ButtonIntClear(USW2);
    pwm_event = 1;
  }
}


/*----------------------------------------------------------------------------
 *      Switch LED on (PROTECTED)
 *---------------------------------------------------------------------------*/
void Switch_On (int led) {
  osMutexAcquire(phases_mut_id, osWaitForever); // try to acquire mutex
  LEDOn(led);
  osMutexRelease(phases_mut_id);
}

/*----------------------------------------------------------------------------
 *      Switch LED off (PROTECTED)
 *---------------------------------------------------------------------------*/
void Switch_Off (int led) {
  osMutexAcquire(phases_mut_id, osWaitForever); // try to acquire mutex
  LEDOff(led);
  osMutexRelease(phases_mut_id);
}

/*----------------------------------------------------------------------------
 *      PWM
 *---------------------------------------------------------------------------*/
void pwm_led(struct selection* select, uint32_t pwm_state) {
  int i;
  for (i = 0; i < select->act_period*2; i++){
    if ((i%10+1)<=pwm_state){
      Switch_On(select->led);
    }
    else{
      Switch_Off(select->led);
    }
  } // for
} // pwm_led



/*----------------------------------------------------------------------------
 *      acionadora: Activates an argument LED
 *---------------------------------------------------------------------------*/
void acionadora (void *argument) {
  struct selection* select = argument;  // allow argument use inside the thread
  uint32_t pwm_state;
  for (;;) {
    // Will only activate if the controler function unblocks this specific task
    osThreadFlagsWait(0x0001, osFlagsWaitAny ,osWaitForever);    /* wait for an event flag 0x0001 */
    // Retrieve correspondent PWM state
    if (osMessageQueueGet(mid_MsgQueue, &pwm_state, NULL, 0U) == osOK){
      pwm_led(select, pwm_state);
      Switch_Off(select->led);
      osDelay(select->act_period);
    }
  }
}

/*----------------------------------------------------------------------------
 *      acionadora: Controls user input and which LED task to call
 *---------------------------------------------------------------------------*/
void controladora (void *argument) {
  osThreadId_t acionador_atual = tid_led1;
  uint32_t pwm = 1;
  
  for (;;) {
    if (pwm_event){
      pwm++;
      if (pwm==11){
        pwm = 1;
      }
      
      pwm_event = 0;
    }
    else  if(led_event){
      if (acionador_atual==tid_led1){
        acionador_atual = tid_led2;
      }
      else if (acionador_atual==tid_led2){
        acionador_atual = tid_led3;
      }
      else if (acionador_atual==tid_led3){
        acionador_atual = tid_led4;
      }
      else if (acionador_atual==tid_led4){
        acionador_atual = tid_led1;
      }
      led_event = 0;
    }
    
    if(osMessageQueuePut(mid_MsgQueue, &pwm, 0U, 0U) == osOK){
      osThreadFlagsSet(acionador_atual, 0x0001);
    }
    
  }
}


/*----------------------------------------------------------------------------
 *      Main: Initialize
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {
  struct selection first = {LED1, 1000};
  struct selection second = {LED2, 1000};
  struct selection third = {LED3, 1000};
  struct selection fourth = {LED4, 1000};
  
  tid_controladora  = osThreadNew(controladora,  NULL, NULL);
  tid_led1 = osThreadNew(acionadora, &first, NULL);
  tid_led2 = osThreadNew(acionadora, &second, NULL);
  tid_led3 = osThreadNew(acionadora, &third, NULL);
  tid_led4 = osThreadNew(acionadora, &fourth, NULL);

  phases_mut_id = osMutexNew(&Phases_Mutex_attr);
  mid_MsgQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(uint8_t), NULL);

  osDelay(osWaitForever);
  while(1);
}

int main (void) {
  // System Initialization
  LEDInit(LED4 | LED3 | LED2 | LED1);
  ButtonInit(USW1);
  ButtonIntEnable(USW1);
  ButtonInit(USW2);
  ButtonIntEnable(USW2);

  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, NULL);    // Create application main thread
  
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }

  while(1);
}
