## Final Project

Use concurrent programming with RTOS, interruptions and integrated peripherals from a microcontroller to implement a control system. The selected project  is a system with three elevators in a 15-story building (total of 16 with the ground floor). The following points highlight the functionalities of the project:
* Simulated environment utilizing the UART interface for communication (see [this subdirectory](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/sim) for the simulator and [this subdirectory](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/docs) for the documentation);
* Complete project development, starting with requirements gathering, moving to modeling, then implementing and finally testing;
* Use of threads, message queues, mutexes and flags for control.

![sim](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/images/elevator_sim.png)

Through the already mentioned UART interface, it is possible to give commands to the simulator utilizing certain predefined carachters (which is detailed in the manual). Similarly, user input is also communicated using strings of carachters, which happens upon interaction with the simulator via buttons representing input from a real life elevator.

The buttons present in the simulator are separated into two major types:
* *Outside buttons*: these are located on each floor, used to call each individual elevator (one for indicating a request to go up and another to go down). This amounts to a total of 84 buttons — considering the top floor (15) only has buttons to go down and the bottom floor (0) only has buttons to go up.
* *Inside buttons*: these are located inside each elevator, to indicate to which floor — from 0 to 15 — should the elevator move to. Considering the three elevators, this amounts to a total of 48 buttons.

### Requirements

The elicitation of requirements is of paramount importance to the success of any project. The following are divided into three groups: functional requirements (which define the applicability of the system), non-functional requirements (which better express the criteria the system should follow) and constraints (which are pre-defined and limit the options for the development the project).

| Functional Requirement  | Description |
| ----------------------- | ----------- |
| FR01  | The system must receive input from the user through the given simulator. |
| FR02  | The system must send output commands to the given simulator. |
| FR03  | The system shall control the elevators independently of one another.  |
| FR04  | The system shall only conduct an elevator between floors given an input on its corresponding section (left, center or right).  |
| FR05  | The system is requested to conduct the correspondent elevator to the correspondent floor when a user presses one of the outside buttons of said floor.  |
| FR06  | The system is requested to conduct the correspondent elevator to a given floor when a user presses one of the inside buttons of said elevator.  |
| FR07  | The system shall indicate to the user that a button was pressed inside a simulated elevator by lighting it up.  |
| FR08  | The system shall turn off the correspondent inside button light once the elevator reaches its destination.  |
| FR09  | The system is requested to open the door of the elevator once this reaches its destination.  |
| FR10  | The system must open elevator doors only when one is not in movement.  |
| FR11  | The system must open elevator doors only when one is leveled with one of the 16 floors. |
| FR12  | The system must ensure an elevator remains stationary with its doors closed given no new input from the user.  |
| FR13  | The system shall perform user input requests in sequential order, given no matching movement pattern.\* |
| FR14  | The system shall prioritize elevator stops at floors in floor order, not in the sequence they were pressed, given the opportunity.\* |
| FR15  | The system shall conduct an elevator to the highest floor given two input commands from outside buttons to go down.\*\* |
| FR16  | The system shall conduct an elevator to the lowest floor given two input commands from outside buttons to go up.\*\* |

\* To further illustrate these functional requirements, consider the following situation which happens in this order:
* Elevator at ground level;
* Inside button for 15th floor is pressed;
* While in movement between the first floors:
    * Inside buttons for floors 14 and 13 are also pressed;
    * Outside button to go up is pressed on the 12th floor;
    * Outside button to go down is pressed on the 10th floor.

The system will not follow the order the buttons were pressed when halting the elevator at the correspondent floors. It **will** in fact identify if any requests can be met earlier, such as the stops at the 13th and 14th floors. Additionally, the request for the 12th floor can also be met earlier, since it already follows the movement direction of the elevator. However, the request on the 10th floor will not be met until the elevator's movement direction is met (downward) or the remaining requests are concluded.

\*\* Now consider the following situation:
* Elevator at ground level;
* Outside button to go down is pressed on the 5th floor;
* Outside button to go down is pressed on the 10th floor.

Again, the system will not answer these requests in sequential order. The elevator will be conducted first to the 10th floor and then, once it is moving downward, answer the request for the 5th floor.


| Non-Functional Requirement  | Description |
| --------------------------- | ----------- |
| NFR01  | The system is requested to initiate the movement of the elevator at most in 500ms upon receiving a new request.  |
| NFR02  | The system is requested to stop an elevator in up to 100ms when reaching a new destination.  |
| NFR03  | The system is requested to open the doors of an elevator in up to 100ms upon stopping at a new destination.  |
| NFR04  | The system must halt an elevator for 3 seconds with its doors open upon reaching a new destination.  |
| NFR05  | The system must ignore when a button is pressed repeatedly for a request that is already being treated. |


| Constraint  | Description |
| ----------- | ----------- |
| C01  | The system is requested to be implemented using the Tiva C Series TM4C1294 microcontroller.  |
| C02  | The system must be implemented using the RTOS Keil RTX5. |
| C03  | The software shall be programmed using the C programming language.  |
| C04  | The system must communicate through the UART interface.  |
| C05  | The system must communicate using only the commands given by the [simulator manual](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/docs/Manual_simulador_elevador.pdf). |


### Project Architecture

Defining the architecture of the project before implementing it ensures that its structure and functionalities are well defined and thought out. This process makes it easier to understand the whole system and makes the decision-making process more efficient.

The following image depicts the object diagram of the system, including the instances to be used — such as objects, threads and data structures. This high level illustration highlights **what** the system effectively has as its objetive.

![sim](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/images/object_diagram.png)

On the other hand. the following behavioural (activity) diagram goes deeper in explaining **how** the system is projected to achieve its objective. This low level illustration requires a more intrinsic understanding of the problem's domain and peculiarities.

![sim](https://github.com/victorlou/embarcados_ELF74/blob/main/projeto_final/images/activity_diagram.png)