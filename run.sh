#!/bin/sh

# $1 - The slug of the exercise (e.g. two-fer).
# $2 - A path to an input directory containing the submitted solution file(s) and the necessary test file(s) (with a trailing slash).
# $3 - A path to an output directory (with a trailing slash).

cwd=$(pwd)
test_file=$(echo "$1" | sed 's/-/_/')_test.cpp
cd "$2" || exit
cmake -DOUTPUT_JUNIT_REPORT=ON -S "$(pwd)" -B /tmp/build
cmake --build /tmp/build
python3 "${cwd}"/process.py /tmp/build/output.xml "$3"results.json
