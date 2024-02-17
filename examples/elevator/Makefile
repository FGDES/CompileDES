# Makefile to compile for the WAGO IPC target
#
# Note: to obtain binaries that execute on the WAGO IPC, 
# your build system must have relevant libraries in the 
# same version as the WAGO IPC. Ubuntu 10.4 LTS works 
# fine in this regard. 

# WAGO provided libraries
LIBWPC = ../libwpc
LIBS = $(LIBWPC)/libkbus.a $(LIBWPC)/libuio.a $(LIBWPC)/wago_ipc_kbus.o

# tools
CDES = ../../build/compiledes

# embedded C code, compile to binary
%_wpc.c: %_wpc.cgc $(CDES)
	$(CDES) -t ec -o $@ $<
%_wpc: %_wpc.c 
	gcc -static -Os -m32 $< -o $@ $(LIBS) -lpthread -lrt
	strip $@

# IEC 61131-3 ST code, to be translated by the GEB toochain
# Note: a WAGO IPC target device required to set-up the 
# GEB toolchein accordingly.
%_geb.st: %_geb.cgc $(CDES)
	$(CDES) -t geb -o $@ $<


# clean
clean:
	rm -rf *_wpc
	rm -rf *_wpc.c
	rm -rf *_geb.st
