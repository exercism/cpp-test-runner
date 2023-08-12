#!/usr/bin/env sh

# Synopsis:
# Run the test runner on a solution.

# Arguments:
# $1: exercise slug
# $2: absolute path to solution folder
# $3: absolute path to output directory

# Output:
# Writes the test results to a results.json file in the passed-in output directory.
# The test results are formatted according to the specifications at https://github.com/exercism/docs/blob/main/building/tooling/test-runners/interface.md

# Example:
# ./bin/run.sh two-fer /absolute/path/to/two-fer/solution/folder/ /absolute/path/to/output/directory/

# If any required arguments is missing, print the usage and exit
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
    echo "usage: ./bin/run.sh exercise-slug /absolute/path/to/two-fer/solution/folder/ /absolute/path/to/output/directory/"
    exit 1
fi

slug="$1"
input_dir="${2%/}"
output_dir="${3%/}"
build_dir="/tmp/${slug}"
compilation_errors_file_name="compilation-errors"
test_output_file_name="test-output.xml"
results_file="${output_dir}/results.json"
binary_file="${build_dir}/${slug}"

# Create the output directory if it doesn't exist
mkdir -p "${output_dir}"

echo "${slug}: testing..."

# Copy the solution to a directory which names matches the slug as
# the makefile uses the directory name to determine the files
cp -R "${input_dir}/" "${build_dir}" && cd "${build_dir}"

cmake -DEXERCISM_TEST_SUITE=1 -DEXERCISM_RUN_ALL_TESTS=1 .
make 2> "${compilation_errors_file_name}"

# In case of compilation errors the executable will not be created
[[ -f "./${slug}" ]] && chmod +x "./${slug}" && "./${slug}" -r xml -o "${test_output_file_name}"
/opt/test-runner/bin/exercism_parser "${build_dir}/${test_output_file_name}" "${results_file}" "${build_dir}/${compilation_errors_file_name}"

cd -

echo "${slug}: done"
