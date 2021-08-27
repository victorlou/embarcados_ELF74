/* -------------------------------------------------------------------------- 
 * UNIVERSIDADE TECNOLOGICA FEDERAL DO PARANA - CURITIBA
 * Created on August 15 2021
 * Author: Victor Feitosa Lourenco  (RA: 1796240)
 *
 * Final project in the Embedded Systems course for the Computer Engineering
 * bachelor degree at UTFPR. Control system for 3 independent elevators using
 * a simulator and UART communication.
 *
 *      

 *---------------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "cmsis_os2.h" // CMSIS-RTOS

// driverlib library
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "system_TM4C1294.h"


#define QUEUE_SIZE 16

osThreadId_t controller_id, elev_left_id, elev_center_id, elev_right_id;
osMutexId_t uart_id;
osMessageQueueId_t left_MsgQueue, center_MsgQueue, right_MsgQueue;

const osThreadAttr_t controller_attr = {
  .name = "Controller"
};

const osThreadAttr_t elev_left_attr = {
  .name = "Left Elevator"
};

const osThreadAttr_t elev_center_attr = {
  .name = "Center Elevator"
};

const osThreadAttr_t elev_right_attr = {
  .name = "Right Elevator"
};


typedef struct {                                // object data type
  char user_input[QUEUE_SIZE];
  struct osMessageQueueAttr_t* queue_left;
  struct osMessageQueueAttr_t* queue_center;
  struct osMessageQueueAttr_t* queue_right;
} CONTROL_ARG_t;

typedef struct {                                // object data type
  char side;    // which of the three elevators
  char pos;     // which of the 16 floors is the elevator currently at
  int state;   //  -1 -> Down,  0 -> Stationary,  1 -> Up
  char command[QUEUE_SIZE];
  struct osMessageQueueAttr_t* queue;
  struct Node* head;
} ELEV_ARG_t;


/*----------------------------------------------------------------------------
 *     Linked list implementation
 *  Used https://www.geeksforgeeks.org/linked-list-set-1-introduction/
 *  for reference.
 *---------------------------------------------------------------------------*/

// A linked list node
struct Node
{
  char floor;
  int direction;  //  -1 -> Down,  0 -> None,  1 -> Up
  struct Node *next;
};
 
/* Given a reference (pointer to pointer) to the head of a list and
   an int, inserts a new node on the front of the list. */
void push(struct Node** head_ref, char new_floor, int new_direction)
{
    /* 1. allocate node */
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
 
    /* 2. put in the data  */
    new_node->floor  = new_floor;
    new_node->direction = new_direction;
 
    /* 3. Make next of new node as head */
    new_node->next = (*head_ref);
 
    /* 4. move the head to point to the new node */
    (*head_ref)    = new_node;
}
 
/* Given a node prev_node, insert a new node after the given
   prev_node */
void insertAfter(struct Node* prev_node, char new_floor, int new_direction)
{
    /*1. check if the given prev_node is NULL */
    if (prev_node == NULL)
    {
      printf("the given previous node cannot be NULL");
      return;
    }
 
    /* 2. allocate new node */
    struct Node* new_node =(struct Node*) malloc(sizeof(struct Node));
 
    /* 3. put in the data  */
    new_node->floor  = new_floor;
    new_node->direction = new_direction;
 
    /* 4. Make next of new node as next of prev_node */
    new_node->next = prev_node->next;
 
    /* 5. move the next of prev_node as new_node */
    prev_node->next = new_node;
}

/* Given a reference (pointer to pointer) to the head
   of a list and an int, appends a new node at the end  */
void append(struct Node** head_ref, char new_floor, int new_direction)
{
    /* 1. allocate node */
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
 
    struct Node *last = *head_ref;  /* used in step 5*/
 
    /* 2. put in the data  */
    new_node->floor  = new_floor;
    new_node->direction = new_direction;
 
    /* 3. This new node is going to be the last node, so make next of
          it as NULL*/
    new_node->next = NULL;
 
    /* 4. If the Linked List is empty, then make the new node as head */
    if (*head_ref == NULL)
    {
       *head_ref = new_node;
       return;
    }
 
    /* 5. Else traverse till the last node */
    while (last->next != NULL)
        last = last->next;
 
    /* 6. Change the next of last node */
    last->next = new_node;
    return;
}

/* Checks whether the value x is present in linked list */
bool search(struct Node* head, char s_floor, int s_direction)
{
    struct Node* current = head; // Initialize current
    while (current != NULL)
    {
      if (current->floor == s_floor && current->direction == s_direction){
        return true;
      }
      current = current->next;
    }
    return false;
}

/*  */
struct Node* maintainDirection(struct Node* head)
{
    struct Node* current = head; // Initialize current
    int initial_direction = head->direction;
    char initial_floor = head->floor;
    
    while (current->next != NULL)
    {
      if (initial_direction == -1){
        // First change of direction happens at a command for same direction, but
        //   at a higher floor
        if (current->next->direction == initial_direction && current->next->floor > initial_floor){
          return current;
        }
        else if(current->next->direction != initial_direction){
          current = current->next;
          // Go through all the opposite direction commands and return the first
          //  that returns to the initial direction
          while(current->next != NULL && current->next->direction != initial_direction){
            current = current->next;
          }
          return current;
        }
      }
      else if (initial_direction == 1){
        // First change of direction happens at a command for same direction, but
        //   at a lower floor
        if (current->next->floor < initial_floor){
          return current;
        }
        else if(current->next->direction != initial_direction){
          current = current->next;
          // Go through all the opposite direction commands and return the first
          //  that returns to the initial direction
          while(current->next != NULL && current->next->direction != initial_direction){
            current = current->next;
          }
          return current;
        }
      }
      current = current->next;
    }
    return current;
}

/*  */
struct Node* changeDirection(struct Node* head)
{
    struct Node* current = head; // Initialize current
    int initial_direction = head->direction;
    char initial_floor = head->floor;
    
    while (current->next != NULL)
    {
      if (initial_direction == -1){
        // First change of direction happens at a command for same direction, but
        //   at a higher floor  OR  literal first change of direction
        if (current->next->floor > initial_floor || current->next->direction != initial_direction){
          return current;
        }
      }
      else if (initial_direction == 1){
        // First change of direction happens at a command for same direction, but
        //   at a lower floor  OR  literal first change of direction
        if (current->next->floor < initial_floor || current->next->direction != initial_direction){
          return current;
        }
      }
      current = current->next;
    }
    return current;
}

/* Given a reference (pointer to pointer) to the head of a
   list and a key, deletes the first occurrence of key in
   linked list */
void pop(struct Node** head_ref)
{
    // Store head node
    struct Node *temp = *head_ref;
 
    *head_ref = temp->next; // Changed head
    free(temp); // free old head
}


/* Function to delete the entire linked list */
void deleteList(struct Node** head_ref)
{
   /* deref head_ref to get the real head */
   struct Node* current = *head_ref;
   struct Node* next;
 
   while (current != NULL)
   {
       next = current->next;
       free(current);
       current = next;
   }
   
   /* deref head_ref to affect the real head back
      in the caller. */
   *head_ref = NULL;
}


/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *      UART definitions
 *---------------------------------------------------------------------------*/
extern void UARTStdioIntHandler(void);

void UARTInit(void){
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Initialize the UART for console I/O (115200bps to work with the simulator)
  UARTStdioConfig(0, 115200, SystemCoreClock);

  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  UARTFlushRx();
  UARTFlushTx(true);
} // UARTInit

void UART0_Handler(void){
  UARTStdioIntHandler();
} // UART0_Handler
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *      Auxiliary functions
 *---------------------------------------------------------------------------*/

void myKernelInfo(void){
  osVersion_t osv;
  char infobuf[16];
  if(osKernelGetInfo(&osv, infobuf, sizeof(infobuf)) == osOK) {
    UARTprintf("Kernel Information: %s\n",   infobuf);
    UARTprintf("Kernel Version    : %d\n",   osv.kernel);
    UARTprintf("Kernel API Version: %d\n\n", osv.api);
    UARTFlushTx(false);
  } // if
} // myKernelInfo

void myKernelState(void){
  UARTprintf("\nKernel State: ");
  switch(osKernelGetState()){
    case osKernelInactive:
      UARTprintf("Inactive\n\n");
      break;
    case osKernelReady:
      UARTprintf("Ready\n\n");
      break;
    case osKernelRunning:
      UARTprintf("Running\n\n");
      break;
    case osKernelLocked:
      UARTprintf("Locked\n\n");
      break;
    case osKernelError:
      UARTprintf("Error\n\n");
      break;
  } //switch
  UARTFlushTx(false);
} // myKernelState

void myThreadState(osThreadId_t thread_id){
  osThreadState_t state;
  state = osThreadGetState(thread_id);
  switch(state){
  case osThreadInactive:
    UARTprintf("Inactive\n");
    break;
  case osThreadReady:
    UARTprintf("Ready\n");
    break;
  case osThreadRunning:
    UARTprintf("Running\n");
    break;
  case osThreadBlocked:
    UARTprintf("Blocked\n");
    break;
  case osThreadTerminated:
    UARTprintf("Terminated\n");
    break;
  case osThreadError:
    UARTprintf("Error\n");
    break;
  } // switch
} // myThreadState

void myThreadInfo(void){
  osThreadId_t threads[8];
  uint32_t number = osThreadEnumerate(threads, sizeof(threads));
  
  UARTprintf("Number of active threads: %d\n", number);
  for(int n = 0; n < number; n++){
    UARTprintf("  %s (priority %d) - ", osThreadGetName(threads[n]), osThreadGetPriority(threads[n]));
    myThreadState(threads[n]);
  } // for
  UARTprintf("\n");
  UARTFlushTx(false);
} // myThreadInfo


char evaluateFloor(char first, char second){
  int floor = (first - '0') * 10 + (second - '0');
  
  switch (floor) {
      case 0:
          return 'a';
      case 1:
          return 'b';
      case 2:
          return 'c';
      case 3:
          return 'd';
      case 4:
          return 'e';
      case 5:
          return 'f';
      case 6:
          return 'g';
      case 7:
          return 'h';
      case 8:
          return 'i';
      case 9:
          return 'j';
      case 10:
          return 'k';
      case 11:
          return 'l';
      case 12:
          return 'm';
      case 13:
          return 'n';
      case 14:
          return 'o';
      case 15:
          return 'p';
      default:
          printf("Out of range");
          return ' ';
  }
} // evaluateFloor


int evaluateDirection(char dir){
  if (dir == 'd'){
    return -1;
  }
  else if (dir == 's'){
    return 1;
  }
  else{
    return 0;
  }
} // evaluateDirection


void openCloseDoor(char side){
  uint32_t tick;
  
  tick = osKernelGetTickCount();

  osMutexAcquire(uart_id, osWaitForever);
  UARTprintf("%ca\n\r", side);
  UARTFlushTx(false);
  osMutexRelease(uart_id);

  osDelayUntil(tick + 3000);

  osMutexAcquire(uart_id, osWaitForever);
  UARTprintf("%cf\n\r", side);
  UARTFlushTx(false);
  osMutexRelease(uart_id);
} //openCloseDoor


int startMovement(char side, char pos, char goal){
  if (pos > goal){
    osMutexAcquire(uart_id, osWaitForever);
    UARTprintf("%cd\n\r", side);
    UARTFlushTx(false);
    osMutexRelease(uart_id);
    //printf("%cd\n\r", side);
    return -1;
  }
  else if (pos < goal){
    osMutexAcquire(uart_id, osWaitForever);
    UARTprintf("%cs\n\r", side);
    osMutexRelease(uart_id);
    //printf("%cs\n\r", side);
    return 1;
  }
  return 0;
} //startMovement

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *      Thread functions
 *---------------------------------------------------------------------------*/
__NO_RETURN void controller(void *arg){
  CONTROL_ARG_t *control = (CONTROL_ARG_t *)arg;
  
  while(1){
    if(UARTRxBytesAvail()){
      UARTgets(control->user_input,sizeof(control->user_input));
      
      if (control->user_input[1] == 'E' || control->user_input[1] == 'I' || isdigit(control->user_input[1])){
        if (control->user_input[0] == 'e'){
          osMessageQueuePut(control->queue_left, control->user_input, 0U, 0U);
        }
        else if (control->user_input[0] == 'c'){
          osMessageQueuePut(control->queue_center, control->user_input, 0U, 0U);
        }
        else if (control->user_input[0] == 'd'){
          osMessageQueuePut(control->queue_right, control->user_input, 0U, 0U);
        }
      }
    }
  } // while
} // controller


__NO_RETURN void elevator(void *arg){
  ELEV_ARG_t *elev = (ELEV_ARG_t *)arg;
  struct Node* walker = NULL;
  char new_floor;
  int new_direction;
  
  // Initialize elevator
  osMutexAcquire(uart_id, osWaitForever);
  UARTprintf("%cr\n\r", elev->side);
  UARTprintf("%cf\n\r", elev->side);
  UARTFlushTx(false);
  osMutexRelease(uart_id);
  
  //printf("%cr\n\r", elev->side);
  //printf("%cf\n\r", elev->side);
  
  while(1){
    // Treats new input from the user by ultimately adding it to the linked list
    if (osMessageQueueGet(elev->queue, elev->command, NULL, 0U) == osOK){
      //printf("  ----  %s and %d\n\r", elev->command, elev->state);
      if (isdigit(elev->command[1])){
        if (isdigit(elev->command[2])){
          elev->pos = evaluateFloor(elev->command[1], elev->command[2]);
        }
        else{
          elev->pos = evaluateFloor('0', elev->command[1]);
        }
      }
      else{
        // Outside button commands
        if (elev->command[1] == 'E'){
          new_floor = evaluateFloor(elev->command[2], elev->command[3]);
          new_direction = evaluateDirection(elev->command[4]);
        }
        // Inside button commands
        else if (elev->command[1] == 'I'){
          new_floor = elev->command[2];
          new_direction = 0;
          
          // Lights up the correspondent button
          osMutexAcquire(uart_id, osWaitForever);
          UARTprintf("%cL%c\n\r", elev->side, new_floor);
          UARTFlushTx(false);
          osMutexRelease(uart_id);
          //printf("%cL%c\n\r", elev->side, new_floor);
        }
        
        // Where should the new command be inserted?
        //  At the start, if the list is empty
        if (elev->head == NULL){
          if (new_direction == 0) {
            if(new_floor > elev->pos){
              push(&elev->head, new_floor, 1);
            }
            else if(new_floor < elev->pos){
              push(&elev->head, new_floor, -1);
            }
            else{
              push(&elev->head, new_floor, 0);
            }
          }
          else{
            push(&elev->head, new_floor, new_direction);
          }
        }
        // Command already in list (do not insert)
        else if (search(elev->head, new_floor, new_direction)) {
          ;
        }
        // Command with no direction of movement (inside button)
        else if (new_direction == 0) {
          if (!search(elev->head, new_floor, elev->head->direction)) {
            if(elev->state == -1){
              // The elevator has not passed this floor yet
              if(elev->pos > new_floor){
                if (elev->head->direction == elev->state){
                  // If the new command is the highest floor in the list
                  if (new_floor > elev->head->floor){
                    push(&elev->head, new_floor, -1);
                  }
                  else{
                    walker = elev->head;
                    while(walker->next != NULL && new_floor < walker->next->floor && walker->next->direction == walker->direction){
                      walker = walker->next;
                    }
                    insertAfter(walker, new_floor, -1);
                  }
                }
                else{
                  // If the new command is the highest floor in the list
                  if (new_floor > elev->head->floor){
                    push(&elev->head, new_floor, -1);
                  }
                  else{
                    push(&elev->head, new_floor, 1);
                  }
                }
              }
              else{
                if (elev->head->direction == elev->state){
                  walker = changeDirection(elev->head);
                  
                  if(walker->next == NULL || new_floor < walker->next->floor){
                    insertAfter(walker, new_floor, 1);
                  }
                  else{
                    while(walker->next != NULL && new_floor > walker->next->floor && walker->next->direction == walker->direction){
                      walker = walker->next;
                    }
                    insertAfter(walker, new_floor, 1);
                  }
                }
              }
            }
            else if(elev->state == 1){
              // The elevator has not passed this floor yet
              if(elev->pos < new_floor){
                if (elev->head->direction == elev->state){
                  // If the new command is the highest floor in the list
                  if (new_floor < elev->head->floor){
                    push(&elev->head, new_floor, 1);
                  }
                  else{
                    walker = elev->head;
                    while(walker->next != NULL && new_floor > walker->next->floor && walker->next->direction == walker->direction){
                      walker = walker->next;
                    }
                    insertAfter(walker, new_floor, 1);
                  }
                }
                else{
                  // If the new command is the highest floor in the list
                  if (new_floor < elev->head->floor){
                    push(&elev->head, new_floor, 1);
                  }
                  else{
                    push(&elev->head, new_floor, -1);
                  }
                }
              }
              else{
                if (elev->head->direction == elev->state){
                  walker = changeDirection(elev->head);
                  
                  if(walker->next == NULL || new_floor > walker->next->floor){
                    insertAfter(walker, new_floor, -1);
                  }
                  else{
                    while(walker->next != NULL && new_floor < walker->next->floor && walker->next->direction == walker->direction){
                      walker = walker->next;
                    }
                    insertAfter(walker, new_floor, -1);
                  }
                }
              }
            }
          }
        }
        // Command in the direction of movement
        else if (elev->state == new_direction) {
          // If the elevator is going down
          if (elev->state == -1){
            // The elevator has not passed this floor yet
            if(new_floor < elev->pos){
              // If the new command is the lowest floor in the list
              if (new_floor > elev->head->floor){
                push(&elev->head, new_floor, new_direction);
              }
              else if (elev->head->direction == elev->state){
                walker = elev->head;
                while(walker->next != NULL && new_floor < walker->next->floor && walker->next->direction == walker->direction){
                  walker = walker->next;
                }
                insertAfter(walker, new_floor, new_direction);
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor > walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
            else{
              if (elev->head->direction == elev->state){
                // If the elevator has already passed it, this request must be at
                //  the end of the list, when the elevator will go down again
                walker = maintainDirection(elev->head);
                while(walker->next != NULL && new_floor < walker->next->floor){
                  walker = walker->next;
                }
                insertAfter(walker, new_floor, new_direction);
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor > walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
          }
          // If the elevator is going up
          else if (elev->state == 1){
            // The elevator has not passed this floor yet
            if(new_floor > elev->pos){
              // If the new command is the lowest floor in the list
              if (new_floor < elev->head->floor){
                push(&elev->head, new_floor, new_direction);
              }
              else if (elev->head->direction == elev->state){
                walker = elev->head;
                while(walker->next != NULL && new_floor > walker->next->floor && walker->next->direction == walker->direction){
                  walker = walker->next;
                }
                insertAfter(walker, new_floor, new_direction);
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor < walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor > walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
            else{
              if (elev->head->direction == elev->state){
                // If the elevator has already passed it, this request must be at
                //  the end of the list, when the elevator will go down again
                walker = maintainDirection(elev->head);
                while(walker->next != NULL && new_floor > walker->next->floor){
                  walker = walker->next;
                }
                insertAfter(walker, new_floor, new_direction);
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor < walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor > walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
          }
        }
        // Command in the opposite direction of movement
        else if (elev->state != new_direction) {
          if (elev->state == 1){
            // The elevator has not passed this floor yet
            if (new_floor > elev->pos){
              if (elev->head->direction != elev->state){
                // If the new command is the lowest floor in the list
                if(new_floor > elev->head->floor){
                  push(&elev->head, new_floor, new_direction);
                }
                else{
                  walker = elev->head;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->next->direction != walker->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor > walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
            else{
              // If the elevator has already passed it, this request must be at
              //  the end of the list, when the elevator will go down again
              if (elev->head->direction != elev->state){
                  walker = elev->head;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->next->direction != walker->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor > walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
          }
          else if (elev->state == -1){
            // The elevator has not passed this floor yet
            if (new_floor < elev->pos){
              if (elev->head->direction != elev->state){
                // If the new command is the lowest floor in the list
                if(new_floor < elev->head->floor){
                  push(&elev->head, new_floor, new_direction);
                }
                else{
                  walker = elev->head;
                  while(walker->next != NULL && new_floor > walker->next->floor && walker->next->direction != walker->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor < walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor > walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
            else{
              // If the elevator has already passed it, this request must be at
              //  the end of the list, when the elevator will go down again
              if (elev->head->direction != elev->state){
                  walker = elev->head;
                  while(walker->next != NULL && new_floor > walker->next->floor && walker->next->direction != walker->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
              }
              else{
                walker = changeDirection(elev->head);
                  
                if(walker->next == NULL || new_floor > walker->next->floor){
                  insertAfter(walker, new_floor, new_direction);
                }
                else{
                  walker = walker->next;
                  while(walker->next != NULL && new_floor < walker->next->floor && walker->direction == walker->next->direction){
                    walker = walker->next;
                  }
                  insertAfter(walker, new_floor, new_direction);
                }
              }
            }
          }
        }
          
        elev->command[0] = '\0';   // reset array
        elev->command[1] = '\0'; 
        elev->command[2] = '\0';
      }
    }
    // Treats commands still not completed in the linked list
    else{
      if (elev->head != NULL){
        
        // Elevator is stationary
        if (elev->state == 0){
          // Means the elevator is already at the correct destination
          if (elev->pos == elev->head->floor){
            osMutexAcquire(uart_id, osWaitForever);
            UARTprintf("%cD%c\n\r", elev->side, elev->pos);
            UARTFlushTx(false);
            osMutexRelease(uart_id);
            //printf("%cD%c\n\r", elev->side, elev->pos);
            
            openCloseDoor(elev->side);
            pop(&elev->head);
          }
          else{
            elev->state = startMovement(elev->side, elev->pos, elev->head->floor);
          }
        }
        // Elevator is in motion
        else{
          // Reached destination
          if (elev->pos == elev->head->floor){
            // Stop elevator
            osMutexAcquire(uart_id, osWaitForever);
            UARTprintf("%cp\n\r", elev->side);
            UARTprintf("%cD%c\n\r", elev->side, elev->pos);
            UARTFlushTx(false);
            osMutexRelease(uart_id);
            //printf("%cp\n\r", elev->side);
            //printf("%cD%c\n\r", elev->side, elev->pos);
            
            elev->state = 0;
            
            //walker = elev->head;
            //while (walker!=NULL){
            //  printf("..... %c   %d\n", walker->floor, walker->direction);
            //  walker = walker->next;
            //}
            
            openCloseDoor(elev->side);
            printf(".");
            pop(&elev->head);
          }
        }
      }
      // REMEMBER TODO: for outside buttons, change direction to match the movement
    }
   
  } // while
} // elevator

/*----------------------------------------------------------------------------*/

void main(void){
  UARTInit();
  //myKernelInfo();
  //myKernelState();
  
  if(osKernelGetState() == osKernelInactive)
     osKernelInitialize();

  // Three linked lists, one for each elevator
  struct Node* h_l = NULL;
  struct Node* h_c = NULL;
  struct Node* h_r = NULL;
  // Three message queues, one for each elevator
  left_MsgQueue = osMessageQueueNew(QUEUE_SIZE, QUEUE_SIZE*sizeof(char), NULL);
  center_MsgQueue = osMessageQueueNew(QUEUE_SIZE, QUEUE_SIZE*sizeof(char), NULL);
  right_MsgQueue = osMessageQueueNew(QUEUE_SIZE, QUEUE_SIZE*sizeof(char), NULL);
  
  CONTROL_ARG_t control_args = { .queue_left=left_MsgQueue,
                                 .queue_center=center_MsgQueue,
                                 .queue_right=right_MsgQueue};
  ELEV_ARG_t elev_left_args = {   .pos= 'a', .side='e', .state=0, .queue=left_MsgQueue, .head=h_l};
  ELEV_ARG_t elev_center_args = { .pos= 'a', .side='c', .state=0, .queue=center_MsgQueue, .head=h_c};
  ELEV_ARG_t elev_right_args = {  .pos= 'a', .side='d', .state=0, .queue=right_MsgQueue, .head=h_r};
  
  //myKernelState();

  controller_id = osThreadNew(controller, &control_args, &controller_attr);
  elev_left_id = osThreadNew(elevator, &elev_left_args, &elev_left_attr);
  elev_center_id = osThreadNew(elevator, &elev_center_args, &elev_center_attr);
  elev_right_id = osThreadNew(elevator, &elev_right_args, &elev_right_attr);
  uart_id = osMutexNew(NULL);
  

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
