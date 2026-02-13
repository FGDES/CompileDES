#!/bin/bash

############################################################################
# convenience script to copy, configure and compile libFAUDES from source

# this script must be invoked with ./DompileDES as the current directory
# and with the source and destination specification relative to that
# path; a typical setup will have ./libFAUDES next to ./CompileDES

# edit this line to select the libFAUDES source location
#FAUDES_SRC=../libfaudes-2_27a
FAUDES_SRC=../libFAUDES

# libFAUDES destination (do not shange)
FAUDES_DST=libFAUDES_source
FAUDES_LIB=libFAUDES_lib
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  FAUDES_LIB=libFAUDES_lx
elif [[ "$OSTYPE" == "darwin"* ]]; then
  FAUDES_LIB=libFAUDES_osx
elif [[ "$OSTYPE" == "msys" ]]; then
  FAUDES_LIB=libFAUDES_msys
fi    



############################################################################
# report to user
echo ==================== copyfaudes.sh
echo "copy, configure and compile libFAUDES for CompileDES project"
echo "current directory: " $(pwd)
echo "obtaining libFAUDES from " $FAUDES_SRC 
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

# cleanup environment
unset FAUDES_PLUGINS
unset FAUDES_DEBUG
unset FAUDES_OPTIONS
unset FAUDES_LINKING
unset FAUDES_PLATFROM

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


############################################################################
# seperate lib and source

rm -rf ${FAUDES_LIB}
mkdir ${FAUDES_LIB}
mv ${FAUDES_DST}/libfaudes.* ${FAUDES_LIB}
cp -R ${FAUDES_DST}/include ${FAUDES_LIB}/

if [ ! -f ${FAUDES_LIB}/libfaudes.a ]; then
    echo ==================== 
    echo "ERROR: something went wrong when compiling libFAUDES: abort"
    return
fi


echo ==================== 
echo "set up to build CompileDES" 
echo "done ... source in " $(pwd)/$FAUDES_DST
echo "done ... libary in " $(pwd)/$FAUDES_DST
