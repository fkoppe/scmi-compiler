#!/bin/bash

BUILD_DIR="build_cli"

if [ -z "$1" ]; then
    echo "Usage: run.sh <your-script.sc>"
    exit 1
fi

SCRIPT_FILE="$1"

if [ ! -f "$SCRIPT_FILE" ]; then
    echo "Error: The script file '$SCRIPT_FILE' does not exist."
    exit 1
fi

cmake -B "./build_cli" -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE="Release"
cmake --build ./$BUILD_DIR

./$BUILD_DIR/scmi_output "$SCRIPT_FILE" >/dev/null

java -jar mi-sim-cli.jar output.mi > output.txt

./$BUILD_DIR/scmi_output output.txt
