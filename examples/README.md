# CodeGenerator --- example configurations


The _blink_ example is a variation of the common microcontroller hellp world test.
The code generator is configured witn automata that mimiques the intended behaviour.
For physicak output, relevant events are associatde with edges of actual outputs.
Variations include typical Arduino devices based on an ATmega or a Freescale K20
MCU as well as an IEC compatible PLC.

| ./blink/*                   | microcontroller hello-world variation            |
| ./blink/blink_atm.cgc       |... target ATmega, e.g., Arduino Nano             |
| ./blink/Makefile_Atm        |... makefile for ATmega target                    |
| ./blink/blink_k20.cgc       |... target Freescale K20, here Teensy 3.2         |
| ./blink/Makefile_K20        |... makefile K20 target                           |
| ./blink/blink_geb.cgc       |... target IEC Structured Text, Gebautomation IDE |


The _elevator_ example controls an actual physical model avialable at the FGDES lab.
Here, hardware IO is implemented by a WAGO IPC.

|./elevator/*                | real world application elevator lab experiment    |


Additional third party sources for conveience (other licensing terms may apply)

|./libk20/*                  |support files for K20/Teensy 3.2 boards |                
|./libwpc/*                  |support files Wago IPC / Kbus drivers   |


 
