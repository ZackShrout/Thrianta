#!/bin/bash
# Build script for rebuilding everything
set echo on

echo "Building everything..."


# pushd engine
# source build.sh
# popd
make -f Makefile.Engine.Linux.mak all

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

# pushd testbed
# source build.sh
# popd

make -f Makefile.Testbed.Linux.mak all
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then

make -f Makefile.Tests.Linux.mak all
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then

echo "All assemblies built successfully."