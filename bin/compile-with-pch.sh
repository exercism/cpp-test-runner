#!/usr/bin/env sh

# Synopsis:
# Compile one file with the C++ compiler. CMake calls this script in place of
# the compiler (see CMAKE_CXX_COMPILER_LAUNCHER in run.sh) and passes the full
# compiler command as arguments.

# The test file includes Catch2's catch_all.hpp, which takes about a second to
# parse. The Docker image holds a precompiled copy of that header, built in
# the Dockerfile. GCC only uses a precompiled header if it is
# the first thing included, and the test file includes the solution first.
# So for the test file, this script includes Catch2 before anything else.
# The test file's own include of Catch2 then does nothing.
#
# The solution's files are compiled unchanged. If a precompiled header does not
# match the compiler options, GCC ignores it and parses the header as before.

# run.sh sets EXERCISM_TEST_FILE to the test file's path. CMake passes the
# source file as the last argument.
for last_arg; do :; done

if [ "${last_arg}" = "${EXERCISM_TEST_FILE}" ]; then
    exec "$@" -include catch2/catch_all.hpp
fi
exec "$@"
