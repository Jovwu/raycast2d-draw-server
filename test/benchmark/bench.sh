#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <source_file>"
    exit 1
fi

SOURCE_FILE=$1
OUTPUT_FILE="a.out"

g++ -Wall -std=c++23 $SOURCE_FILE -pthread -lbenchmark -msse4.1 -mavx -mfma -O3 -o $OUTPUT_FILE

if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

./$OUTPUT_FILE

if [ $? -ne 0 ]; then
    echo "Execution failed"
    exit 1
fi

rm -f $OUTPUT_FILE

echo "Benchmark completed and executable removed."