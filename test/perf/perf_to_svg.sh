#!/bin/bash

PROCESS_NAME="jovwuserver"
FLAMEGRAPH_DIR="/home/jovwu/Documents/third-lib/FlameGraph"

PID=$(pgrep $PROCESS_NAME)
if [ -z "$PID" ]; then
    echo "Failed to get PID of $PROCESS_NAME"
    exit 1
fi

echo "------------------ perf record ---------------------"
sudo perf record -e cpu-clock --call-graph dwarf -g -p $PID &

PERF_PID=$!

cleanup() {
    echo "------------------ stopping perf -------------------"
    sudo kill -INT $PERF_PID
    wait $PERF_PID
}


trap cleanup SIGINT

wait $PERF_PID

PERF_DATA="perf.data"
if [ ! -f "$PERF_DATA" ]; then
    echo "Error: perf.data file not found"
    exit 1
fi

sudo perf script -i "$PERF_DATA" &> perf.unfold
if [ $? -ne 0 ]; then
    echo "Error: Failed to generate perf.unfold"
    exit 1
fi

sudo $FLAMEGRAPH_DIR/stackcollapse-perf.pl perf.unfold &> perf.folded
if [ $? -ne 0 ]; then
    echo "Error: Failed to generate perf.folded"
    exit 1
fi

sudo $FLAMEGRAPH_DIR/flamegraph.pl perf.folded > perf.svg
if [ $? -ne 0 ]; then
    echo "Error: Failed to generate perf.svg"
    exit 1
fi

rm -f perf.data perf.unfold perf.folded perf.data.old

echo "Flame graph generated successfully: perf.svg"