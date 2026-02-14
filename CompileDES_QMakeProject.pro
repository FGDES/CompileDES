#############################################################
#
# qmake project file for libFAUDES application CompileDES
#
# Copyright 2016-2025 Thomas Moor
#
#

# set version number
DEFINES += "COMPILEDES_VERSION=\\\"3.14\\\""

# location of libFAUDES
unix: LIBFAUDES = ./libFAUDES_lx
macx: LIBFAUDES = ./libFAUDES_osx
win32: LIBFAUDES = ./libFAUDES_win

# C++ dources and headers
SOURCES += src/cgp_eventconfig.cpp
SOURCES += src/cgp_codegenerator.cpp
SOURCES += src/cgp_codeprimitives.cpp
SOURCES += src/cgp_embeddedc.cpp
SOURCES += src/cgp_iec61131st.cpp
SOURCES += src/cgp_gebtools.cpp
SOURCES += src/cgp_plcoxml.cpp
SOURCES += src/cgp_atmega.cpp
SOURCES += src/cgp_kinetis.cpp
SOURCES += src/compiledes.cpp

HEADERS += src/cgp_eventconfig.h 
HEADERS += src/cgp_include.h
HEADERS += src/cgp_codegenerator.h
HEADERS += src/cgp_codeprimitives.h
HEADERS += src/cgp_embeddedc.h
HEADERS += src/cgp_iec61131st.h
HEADERS += src/cgp_gebtools.h
HEADERS += src/cgp_plcoxml.h
HEADERS += src/cgp_atmega.h
HEADERS += src/cgp_kinetis.h

# other relevant files
DISTFILES += data/blink_atm.cgc 
DISTFILES += data/blink_k20.cgc 
DISTFILES += data/blink_geb.cgc 

#############################################################
#
# this should be all to it ... nothing to be changed below
#
#

# target setting for C++ console application
TEMPLATE = app
CONFIG += console 
CONFIG -= qt
CONFIG -= app_bundle

# dont overwrite other makefiles
MAKEFILE = Makefile_by_QMake

# destination for object files
OBJECTS_DIR = build/obj

# destination for binary
linux: TARGET = build/compiledes_lx
macx:  TARGET = build/compiledes_osx
macx:  TARGET = build/compiledes_ein

# name of static libFAUDES archive
unix:LIBFAUDES_LIB = $$LIBFAUDES/libfaudes.a
win32:LIBFAUDES_LIB = $$LIBFAUDES/faudes.lib

# libFAUDES include path
LIBFAUDES_INC = $$LIBFAUDES/include

# test for existence of relevant libFAUDES files
!exists( $$LIBFAUDES ):error( "missing libFAUDES installation at " $$LIBFAUDES " --- consult online documentation")
!exists( $$LIBFAUDES_INC/libfaudes.h ):error( "broken libFAUDES installation at " $$LIBFAUDES " --- consult online documentation")
!exists( $$LIBFAUDES_LIB ):error( "missing library " $$LIBFAUDES_LIB " --- consult online documentation")
!exists( $$OBJECTS_DIR ):error( "missing output directory " $$OBJECTS_DIR " --- please create")

# direct qmake to statically link with lidFAUDES
LIBS += $$LIBFAUDES_LIB
INCLUDEPATH += $$LIBFAUDES_INC

# extra libraries used in MS Windows
win32:LIBS += wsock32.lib winmm.lib

# extra libraries used with Linux
unix: !macx: LIBS += -lpthread

# extra configuration for Mac Os X
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11


