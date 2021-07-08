## Lab 02 - Exceptions (Execution Analysis)

This lab focuses solely on analysis and configuration alterations to the “simple_io_main_sp” and the “simple_io_process_sp” projects present in the workspace “TM4C1294_Bare_IAR9” (from the previously mentioned [repository]( https://github.com/ELF74-SisEmb/TM4C1294_Bare_IAR9)):
* Disassembly breakpoint, stack and register analysis;
* Force Floating Point Unit to be utilized and then reanalyze;
* Compare results from “simple_io_main_sp” and the “simple_io_process_sp”.

### Conclusion

After the exception that takes the execution of the program to the *SysTick_Handler* function, the top of the stack is composed of values from specific registers that were saved when the exception happened, namely, R0, R1, R2, R3, R12, LR, PC and xPSR (APSR | IPSR | EPSR). It is important to note that the return from the exception to the main flow of code happens utilizing the values in the stack (more specifically the PC, which was saved earlier), rather than the LR register.

Additionally, since the code is now running in handler mode (not thread mode), the stack pointer is referring to the MSP by default (bit 1 of the CONTROL register is set to zero). By forcing a higher priority exception to happen — such as NMI — an ISR preemption will happen, stopping the current exception execution to allow for the execution of the higher priority exception. This means that a second set of registers (the same as before) will be stacked on top of the previous ones, allowing the program to finish the NMI execution, return to the SysTick exception to execute it and then finally return to the main code.

Using the *simple_io_process*, the PSP (bit 1 of the CONTROL register is set to one) is used in thread mode, rather than the MSP (which was the case for the *simple_io_main*). This makes it so that to be able to see the values stacked during the exceptions using the IAR software, it is necessary to look directly at the memory (where the stack pointer is indicating).
