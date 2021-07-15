## Lab 04 - RTOS Introduction

This lab is an introduction to using RTOS, based on the project “tarefas” in the workspace “TM4C1294_RTOS_IAR9-main” (from the previously mentioned [repository (RTOS)](https://github.com/ELF74-SisEmb/TM4C1294_RTOS_IAR9)):
* RTX_Config.h header configuration;
* Thread activation periods;
* Function generalization.

### Conclusions

The OS_TICK_FREQ, OS_THREAD_NUM and OS_THREAD_DEF_STACK_NUM repectively correxpond to the base time unit for delays and timeouts in Hz, the maximum number of user threads that can be active at the same time, and the maximum number of user threads with default stack size.

The functions in this file differ only in how they implement the delay to the blinking LED (one focusing on a constant delay and the other on a constant activation period of the thread). Manipulating the *void &ast;arg* argument of the function, it is possible to generalize it to allow for the activation of any LED with a customizable activation period.