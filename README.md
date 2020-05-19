# Exercism C++ Test Runner

Docker image used for automatic testing of the C++ track's exercises.

This test runner, much like the ones for other tracks, **must** respect the [specified interface][test-runner-interface].

Since the test runners are deployed as Docker images the [related specification][test-runner-docker] **must** be respected for the Dockerfile.

# Running tests

To run tests:

1. Open project's root in terminal
2. Run `./test.sh`

This will compile and run tests for all exercises in the `tests` folder, it will fail if the output file `result.json` is different from the expected one for the exercise.

If you want to run specific tests:

1. Open project's root in terminal
2. Run `./test.sh <EXERCISE_SLUG_1> <EXERCISE_SLUG_2>`

In both cases you can show more informations, like Docker output, by using the flag `-v` or `--verbose`.

# How it works

On the C++ track we use CMake for building the exercises and [Catch][catch-lib] as its test framework.

Catch can report the tests results in [JUnit][junit] formatted xml when enabled, we do so during the config step in `run.sh` by settings the variable `OUTPUT_JUNIT_REPORT`, when this is done a file `output.xml` is generated during build time.

This file is parsed with Python and the [junitparser][junitparser-lib] library in the `process.py` script that outputs a `results.json` file that respects the test runners specifications.

[test-runner-interface]: https://github.com/exercism/automated-tests/blob/master/docs/interface.md
[test-runner-docker]: https://github.com/exercism/automated-tests/blob/master/docs/docker.md
[cmake]: https://cmake.org/
[catch-lib]: https://github.com/catchorg/Catch2
[junit]: https://junit.org/junit5/
[junitparser-lib]: https://github.com/gastlygem/junitparser