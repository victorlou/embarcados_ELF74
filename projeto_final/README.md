## Final Project

Use concurrent programming with RTOS, interruptions and integrated peripherals from a microcontroller to implement a control system. The selected project  is a system with three elevators in a 15-story building. The following points highlight the functionalities of the project:
* Simulated environment utilizing the UART interface for communication (see [this subdirectory](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/sim) for the simulator and [this subdirectory](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/docs) for the documentation);
* Complete project development, starting with requirements gathering, moving to modeling, then implementing and finally testing;
* Use of threads, message queues, mutexes and flags for control.

![sim](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/images/elevator_sim.png)

### Requirements

The elicitation of requirements is of paramount importance to the success of any project. The following are divided into two groups: functional requirements (which define the applicability of the system) and non-functional requirements (which better express the criteria the system should follow).

| Functional Requirement  | Description   |
| ----------------------- | ------------- |
| FR01  | The elevators shall work independently of one another.  |
| FR02  | The system must open elevator doors only when one is not in movement.  |
| FR03  | The system must open elevator doors only when one is leveled with one of the 15 floors. |
| FR04  | The system is required to communicate with the user using a UART interface. |
| FR05  | The system is required to receice information from the user using a UART interface. |
| FR06  | An elevator shall only move between floors given an input on its corresponding section (left, center or right).  |
| FR07  | Each elevator is required to use a message queue for command input.  |
| FR08  | Each elevator is required to request access to a mutex on command output.   |


| Non-Functional Requirement  | Description   |
| ----------------------- | ------------- |
| NFR01  | The system is requested to be implemented using the Tiva C Series TM4C1294 microcontroller.  |
| NFR02  | The system must be implemented using the RTOS Keil RTX5. |
| NFR03  | The software shall be programmed using the C programming language.  |
| NFR04  | The system must be functional with the provided Unity elevator simulator.  |