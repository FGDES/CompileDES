#!/bin/bash

# manually configure versionised package name
PACKAGE=faudes_compiledes_3_14

############################################################################
# convenience script to create an archieve of the code-generator project

echo "==================== makepackage.sh"
echo "create tgz archieve of compiledes"
echo "current directory: " $(pwd)
echo "package name: " $PACKAGE
echo "press return to proceed or ctrl-c to bail out"
read


############################################################################
# some consistency tests
if [ ! -f src/cgp_codegenerator.cpp ]; then
    echo ==================== 
    echo "ERROR: the current directory appears not to be a CompileDES distribution: abort"
    return
fi
if [ ! -f examples/blink/blink_atm.cgc ]; then
    echo ==================== 
    echo "ERROR: the current directory appears not to be a CompileDES distribution: abort"
    return
fi


############################################################################
# package


# safe some disk space
make -f Makefile_GnuMake clean
make -C libFAUDES_source clean
make -C examples/blink -f Makefile_Atm clean
make -C examples/blink -f Makefile_K20 clean
make -C examples/elevator clean
rm -rf libFAUDES_source/bin/*
rm -rf libFAUDES_source/tools
rm -rf libFAUDES_source/stdflx

# set destination
DEST=$(pwd)/${PACKAGE}

echo ==================== prepare destination ${PACKAGE}
rm -rf $DEST
mkdir $DEST
cp -R libFAUDES_* $DEST/
cp -R src $DEST/
cp -R bin $DEST/
cp -R doc $DEST/
cp -R examples $DEST/
cp -R build $DEST/
cp  Makefile* $DEST/
cp  CompileDES_*  $DEST/
cp  *.sh $DEST/
cp  *.bat $DEST/
cp  LICENSE README.md $DEST/
cp  TAR_EXCLUDES  $DEST/


echo ====================compile package
tar -cvzf $DEST.tar.gz --exclude-from=./TAR_EXCLUDES $PACKAGE


echo ======================================== done


