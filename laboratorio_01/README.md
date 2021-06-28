## Lab 01 - Control Familiarization (Blinking LED)

Based on the project “simple_io_main_sp” in the workspace “TM4C1294_Bare_IAR9” (from the previously mentioned [repository]( https://github.com/ELF74-SisEmb/TM4C1294_Bare_IAR9)):
* CPU clock frequency (PLL): 24MHz;
* C compiler optimization level: low (low);
* LED D4 should change state every 1s;
* Timing must be done by software (delay loops), that is, without the use of any hardware interrupt mechanism.

### Conclusion

Switching the optimization of the C compiler has a direct influence on the performance of each cycle. For instance, with the "Medium" setting it is possible to observe the LED blinking at a rate close to once per second, however, switching the setting to "High" makes it blink faster.

Regarding the clock frequency, it is natural that the higher its value, the faster the microcontroller will be able to perform instructions. Nevertheless, the LED will only blink faster if the value of the timer (set in the Reference line of the main code above) does not change accordingly. If it does — proportionally to how the clock frequency is changed — then the LED will maintain its blinking frequency (give or take slight differences due to the compiler optimization).
