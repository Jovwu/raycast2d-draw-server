#!/bin/bash

BUILD_DIR="build"
EXECUTABLE="bin/jovwuserver"
NUM_CORES=16
BUILD_TYPE="Debug"

handle_error() {
    echo "Error occurred in script at line: $1"
    exit 1
}

trap 'handle_error $LINENO' ERR

mkdir -p ${BUILD_DIR}

cd ${BUILD_DIR}

echo "------------------ checking build directory ------------------"
if [ "$(ls -A .)" ]; then
    echo "Build directory exists and is not empty. Cleaning build..."
    make clean
else
    echo "Build directory is empty. Skipping clean."
fi

echo "------------------ running cmake -------------------"
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..

echo "------------------ make start ----------------------"
make -j${NUM_CORES}
# make -j${NUM_CORES} > build_log.txt 2>&1

echo "------------------ make done -----------------------"

echo "-------------------   run   ------------------------"
sudo ../${EXECUTABLE}