#!/bin/sh

# $1 - The slug of the exercise (e.g. two-fer).
# $2 - A path to an input directory containing the submitted solution file(s) and the necessary test file(s) (with a trailing slash).
# $3 - A path to an output directory (with a trailing slash).

cwd=$(pwd)
test_file=$(echo "$1" | sed 's/-/_/')_test.cpp
cd "$2" || exit
cmake -S "$(pwd)" -B /tmp/build
cd /tmp/build
make 2> compilation-errors
# In case of compilation errors the executable will not be created
[[ -f "$1" ]] && ./"$1" -r junit -o test-output.xml
python3 "${cwd}"/process.py /tmp/build/compilation-errors /tmp/build/test-output.xml "$3"results.json
