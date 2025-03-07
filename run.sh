#!/bin/bash

# Navigate to the directory where 'scmi' is located
cd cmake-build-debug || exit 1

# Execute the './scmi' command (no console print)
./scmi > /dev/null 2>&1

# Navigate back to the original directory (optional, if you want to be back in the starting directory)
cd - > /dev/null 2>&1 || exit 1

# Execute the Java command and process the output
output=$(java -jar mi-sim-cli.jar ./output.mi | grep "R12:" | grep -E "R12: [0-9]+ -> [0-9]+;" | awk -F'-> ' '{print $2}' | awk -F';' '{print $1}')

# Convert numbers to ASCII and print without line breaks
echo "$output" | awk '{printf "%c", $1} END {print ""}'
