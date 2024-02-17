###########################################################

Kinetis K20 support files

###########################################################

This directory organises sources to support the stand-alone
compilation of CompileDES generated code for K20 target devices.

The only device actually tested is a Teensy 3.2 board. The
provided support files are stripped down versions of the 
Teensyino extension to the Arduino IDE. Any other K20 series
device will require careful examination/adaption of the provided 
support code. 

Copyright of the original sources is with PJRC.COM, LLC. We 
understand that the license allows redistribution of copyedited
variations. See header files for detailed license information.

Files:

  mk20dx256.h        definition of register mapping as needed
  mk20dx256.c        minimal setup deveice (bus clock,interrupt vector)
  mk20dx256.ld       linker configuration
  usb_serial.h/c     merged files for usb on serial support (debugging only)
