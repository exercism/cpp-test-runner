#!/bin/bash

set -e

cd "$(dirname "$0")"

# Get arguments
POSITIONAL=()
while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -v|--verbose)
        VERBOSE=YES
        ;;
        *)
        POSITIONAL+=("$key")
    esac
    shift
done

# Function that runs a single test
run_test() {
    local SLUG="$1"
    local EXERCISE_PATH="tests/$SLUG"

    echo "Running test $SLUG..."
    if [[ -n $VERBOSE ]]; then
        docker build -t test-runner .
        docker run --rm -v "$(pwd)"/"$EXERCISE_PATH":/mnt/"$EXERCISE_PATH" -v "$(pwd)"/output:/output test-runner "$SLUG" /mnt/"$EXERCISE_PATH"/ /output/
    else
        docker build -t test-runner . > /dev/null
        docker run --rm -v "$(pwd)"/"$EXERCISE_PATH":/mnt/"$EXERCISE_PATH" -v "$(pwd)"/output:/output test-runner "$SLUG" /mnt/"$EXERCISE_PATH"/ /output/ > /dev/null
    fi
    diff output/results.json "$EXERCISE_PATH"/expected_results.json
    echo "Passed"
}

# Iterate slugs to run tests
for INDEX in "${!POSITIONAL[@]}"; do
    SLUG="${POSITIONAL[$INDEX]}"
    if [[ -d "tests/$SLUG" ]]; then
        run_test $SLUG
    else
        echo "No test found for slug $SLUG in tests folder"
        exit 1
    fi
done

# We already run tests given the arguments, nothing else to do
if [[ ${#POSITIONAL[@]} -gt 0 ]]; then
    echo "Done"
    exit 0
fi

# Run tests for all folders found
for i in tests/*; do
    SLUG=$(basename "$i")
    run_test $SLUG
done
echo "Done"
