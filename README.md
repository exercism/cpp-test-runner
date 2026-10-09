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

Catch can report the tests results in [JUnit][junit] formatted xml when enabled, we do so during the config step in `run.sh` by running the built executable with certain flags, when this is done a file `output.xml` is generated during build time.

This file is parsed with Python and the [junitparser][junitparser-lib] library in the `process.py` script that outputs a `results.json` file that respects the test runners specifications.

## The client-side runner

The same `bin/run.sh` also runs in the browser, on a wasm kernel that provides a real Linux userland.
There is no second implementation: the kernel's sysroot carries the toolchain, this repo's `bin/run.sh` is untarred into `/opt/test-runner`, and executed exactly as the Docker image runs it.

`clientside.json` is everything this track says about that:

| Key | Meaning |
| --- | --- |
| `sysroot` | which published sysroot to run on: the version it carries, and an id to tell rebuilds of the same version apart |
| `kernel` | which kernel build to run on |
| `timeout` | seconds a single run may take before the worker is killed |
| `env` | environment variables this track needs on top of the sysroot's own |
| `preload` | binaries to load at boot rather than fault in on first use |

Kernels and sysroots are published from [exercism/clientside-tooling][tooling].
The `kernel` and `sysroot` values are directory names in that repo.

To build the tarball locally:

```bash
./bin/build-clientside-tarball.sh test-runner.tar
```

It needs GNU tar, because the archive has to be byte-identical between runs: the published path is derived from its hash.

Publishing happens in `.github/workflows/publish-clientside.yml`, which runs after a successful Deploy so that the Docker image and the browser tarball always come from the same commit.

[test-runner-interface]: https://exercism.org/docs/building/tooling/test-runners/interface
[test-runner-docker]: https://exercism.org/docs/building/tooling/test-runners/docker
[cmake]: https://cmake.org/
[catch-lib]: https://github.com/catchorg/Catch2
[junit]: https://junit.org/junit5/
[junitparser-lib]: https://github.com/gastlygem/junitparser
[tooling]: https://github.com/exercism/clientside-tooling
