#!/usr/bin/env bash
set -e

# Synopsis:
# Build bin/exercism_parser for the browser test runner, by compiling it inside
# the wasm kernel with the sysroot's own clang and boost.
#
# This is the browser's equivalent of the Dockerfile's
#
#     RUN g++ -I ./include ./src/*.cpp -o ./bin/exercism_parser ...
#
# The kernel's binaries are wasm modules dynamically linked against the
# sysroot's libc, so the parser has to be built by the toolchain it will run
# on. The sysroot's compiler targets the kernel by default, so no cross
# toolchain is needed on the host: the sources go in, the binary comes back out.
#
# The result is then shrunk with binaryen's wasm-opt on the host.
#
# Needs node, `playwright` resolvable from the harness directory (see
# run-in-kernel.mjs), and wasm-opt.
#
# Arguments:
# $1: directory holding kernel.js, kernel_bg.wasm and kernel_client.mjs
# $2: directory holding sysroot.tar
# $3: the merged boot.json
# $4: path to write the parser to (default: bin/exercism_parser)
#
# Environment:
# HARNESS: directory holding run-in-kernel.mjs (default: .github/clientside)
#
# Example:
# ./bin/build-clientside-parser.sh kernel/<id> sysroot/cpp/<version>/<id> boot.json

# Runs inside the kernel, in /opt/test-runner.
#
# The sysroot's clang links every executable with --export-all, which keeps
# every function alive as an export: the parser comes out at 1.6MB. So the
# link is done by hand: clang is asked for its own wasm-ld command (-###),
# --export-all is dropped, and only main is exported. main has to be, because
# the start-up code finds it through the module's exports; nothing else does.
# With wasm-opt afterwards, that is about 55KB.
#
# No -lboost_system, unlike the Dockerfile: it has been header-only since
# Boost 1.69, and the sysroot does not carry the stub library.
# shellcheck disable=SC2016
IN_KERNEL='
set -e
mkdir -p bin obj
for source in src/*.cpp; do
    object="obj/$(basename "${source}" .cpp).o"
    clang++ -Oz -I ./include -c "${source}" -o "${object}"
done
link="$(clang++ -### obj/*.o -o bin/exercism_parser -lboost_json 2>&1 | tail -n 1)"
link="\"${link#*\"}"                      # drop the leading " (in-process) "
link="${link//\"--export-all\" /}"
eval "${link} --export=__main_argc_argv"
'

main() {
    if [[ $# -lt 3 ]]; then
        echo "usage: ./bin/build-clientside-parser.sh <kernel-dir> <sysroot-dir> <boot.json> [output]" >&2
        exit 1
    fi

    local kernel="$1" sysroot="$2" boot="$3" output="${4:-bin/exercism_parser}"
    local harness="${HARNESS:-.github/clientside}"

    if ! command -v wasm-opt > /dev/null; then
        echo "wasm-opt is required (binaryen)" >&2
        exit 1
    fi

    local staging
    staging="$(mktemp -d)"
    # shellcheck disable=SC2064
    trap "rm -rf '${staging}'" EXIT

    # The sources, laid out where the Dockerfile compiles them.
    tar --create --file "${staging}/src.tar" --transform 's|^|opt/test-runner/|' include src

    node "${harness}/run-in-kernel.mjs" \
        --extract /opt/test-runner/bin/exercism_parser "${output}" \
        "${kernel}" "${sysroot}" "${boot}" "${staging}/src.tar" \
        /usr/bin/bash -c "${IN_KERNEL}"

    # Deterministic, like the compile, so the published tarball's hash only
    # changes when the parser does.
    wasm-opt -Oz "${output}" -o "${output}"

    # run.sh calls it directly, and the kernel's file mode does not come out
    # with the bytes.
    chmod +x "${output}"
}

main "$@"
