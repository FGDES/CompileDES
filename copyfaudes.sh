#!/bin/bash

############################################################################
# convenience script to copy, configure and compile libFAUDES from source

# this script must be invoked with ./compiledes as the current directory
# and with the source and destination specification relative to that
# path; a typical setup will have ./libfaudes next to ./compiledes

# edit this line to select the libfaudes source location
#FAUDES_SRC=../libfaudes-2_27a
FAUDES_SRC=../libfaudes

# do not change the libfaudes destination
FAUDES_DST=libfaudes_source


############################################################################
# report to user
echo ==================== copyfaudes.sh
echo "copy, configure and compile libFAUDES for CompileDES project"
echo "current directory: " $(pwd)
echo "obtaining libFAUDES from " $FAUDES_SRC 
# note: comment out these lines if you dont want to use LSB compilers
if [ $(uname -s) == "Linux" ]; then
if [ -d /opt/lsb ]; then
export FAUDES_PLATFORM="lsb_linux"
echo override FAUDES_PLATFORM to use lsb-compilers
fi
fi
echo "press return to proceed or ctrl-c to bail out"
read

############################################################################
# some consistency tests
if [ ! -f src/cgp_codegenerator.cpp ]; then
    echo ==================== 
    echo "ERROR: the current directory appears not to be a CompileDES distribution: abort"
    return
fi
if [ ! -f ${FAUDES_SRC}/src/cfl_baseset.h ]; then
    echo ==================== 
    echo "ERROR: the specified source appears to be not a libFAUDES source tree: abort"
    return
fi


############################################################################
# do it

# configure libFAUDES to go with CompileDES 
# - minimum plug-ins: timed, simulator, iodevice)
# - recommended debug: core_checked core_progress
# - minimum options: core_systime core_network core_thread
# note: change the "static" to your needs, e.g., "static debug"
export FAUDES_PLUGINS="timed iodevice simulator"
export FAUDES_DEBUG="core_checked core_exceptions iop_threads iop_performance sim_sync core_progress"
export FAUDES_OPTIONS="core_systime core_network core_threads"
export FAUDES_LINKING="static"


# do copy/clean/configure/compile
echo ==================== copy source tree
rm -rf $FAUDES_DST
cp -R $FAUDES_SRC $FAUDES_DST 
echo ==================== clean libFAUDES
make -C $FAUDES_DST dist-clean
echo ==================== configure libFAUDES
make -C $FAUDES_DST configure 
echo ==================== build libFAUDES static archieve
make -C $FAUDES_DST -j 20 libfaudes

# safe some disk space
echo ==================== clean source tree
make -C $FAUDES_DST clean
rm -rf $FAUDES_DST/plugins/*
cp -R $FAUDES_SRC/plugins/timed $FAUDES_DST/plugins 
cp -R $FAUDES_SRC/plugins/iodevice $FAUDES_DST/plugins 
cp -R $FAUDES_SRC/plugins/simulator $FAUDES_DST/plugins 
rm -rf $FAUDES_DST/plugins/iodevice/wago/bin/*
rm -rf $FAUDES_DST/doc
rm -rf $FAUDES_DST/tools
rm -rf $FAUDES_DST/src/doxygen/*
rm -rf $FAUDES_DST/plugins/*/src/doxygen/*
rm -rf $FAUDES_DST/bin/*
rm -rf $FAUDES_DST/stdflx
rm -rf $FAUDES_DST/tutorial

# cleanup environment
unset FAUDES_PLUGINS
unset FAUDES_DEBUG
unset FAUDES_OPTIONS
unset FAUDES_LINKING
unset FAUDES_PLATFROM


############################################################################
# done

if [ ! -f ${FAUDES_DST}/libfaudes.a ]; then
    echo ==================== 
    echo "ERROR: something went wrong when compiling libFAUDES: abort"
    return
fi

echo ==================== 
echo "set up to build CompileDES" 
echo "done ... inspect results in " $(pwd)/$FAUDES_DST
