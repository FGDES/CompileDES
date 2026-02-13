#!/bin/bash

############################################################################
# convenience script to compile libFAUDES from provided source

# this script must be invoked within ./CompileDES as the current directory
# and with a configured libFAUDES source tree at ./CompileDES/libFAUDES_source

# libFAUDES sources
FAUDES_SRC=./libFAUDES_source

# libFAUDES destination 
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
echo ==================== buildfaudes.sh
echo "building libFAUDES for CompileDES project"
echo "current directory: " $(pwd)
echo "libFAUDES sources: " $FAUDES_SRC 
echo "build detination: " $FAUDES_LIB 
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

# libFAUDES linking "static" or "static debug"
export FAUDES_LINKING="static"


# do clean/compile
echo ==================== clean libFAUDES
make -C $FAUDES_SRC clean
echo ==================== build libFAUDES static archieve
make -C $FAUDES_SRC -j 20 libfaudes

# cleanup environment
unset FAUDES_LINKING

# harvest results
rm -rf ${FAUDES_LIB}
mkdir ${FAUDES_LIB}
mv ${FAUDES_DST}/libfaudes.* ${FAUDES_LIB}
cp -R ${FAUDES_DST}/include ${FAUDES_LIB}/


# safe some disk space
echo ==================== clean source tree
make -C $FAUDES_SRC clean
rm -rf $FAUDES_SRC/plugins/iodevice/wago/bin/*
rm -rf $FAUDES_DST/bin/*


############################################################################
# done

if [ ! -f ${FAUDES_LIB}/libfaudes.a ]; then
    echo ==================== 
    echo "ERROR: something went wrong when compiling libFAUDES: abort"
    return
fi

echo ==================== 
echo "set up to build CompileDES" 
