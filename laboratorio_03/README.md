## Lab 03 - Finite State Machines (FSM)

This lab applies concepts of finite state machines in four separate exercises focusing on the following topics:
* Building an FSM based on constraints;
* Utilize knowledge from modes of operation to create a FSM;
* Create an FSM using the C programming language.

### Exercise 1

Setting the PRIMASK and FAULTMASK registers to 1 (one) guarantees that no exceptions (other than NMI) can be activated, essentially limiting the execution to thread mode. Furthermore, the CONTROL register on 0 (zero) indicates the MSP swill be used and further ensures the thread mode execution.

![FSM](https://github.com/victorlou/embarcados_ELF74/blob/main/laboratorio_03/mef/ex1.png)

### Exercise 2

Simple execution of a blinking LEDc controlled by an exception routine.

![FSM](https://github.com/victorlou/embarcados_ELF74/blob/main/laboratorio_03/mef/ex2.png)

### Exercise 3 (fsm_state)

To create an FSM based on the Grey Code, the simplest method was to use one state for each binary configuration of the coding. This solution was based on the *"fsm_state"* directory from the previously mentioned [repository]( https://github.com/ELF74-SisEmb/TM4C1294_Bare_IAR9)).

### Exercise 4

A more complex FSM, since it requires the use of hierarchy to be completed. The flow of the code is controlled by two separate interruptions, one caused periodically by the Sys_Tick of the system and one caused by a user input recognized by the press of a button.

![FSM](https://github.com/victorlou/embarcados_ELF74/blob/main/laboratorio_03/mef/ex4.png)
