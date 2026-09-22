#! /bin/bash -e

# Synopsis:
# Build the tarball that the browser test runner untars into its kernel.
#
# This is the browser's equivalent of the Dockerfile's
#
#     WORKDIR /opt/test-runner
#     COPY . .
#
# The kernel's sysroot carries the toolchain but not this runner, so the
# runner ships as a tarball that is unpacked at boot. Staging it here rather
# than tarring the repo directly means the archive's layout is exactly what the
# kernel should end up with, and nothing that only makes sense on a developer's
# machine goes along for the ride.
#
# Arguments:
# $1: path to write the tarball to (default: ./test-runner.tar)
#
# Example:
# ./bin/build-clientside-tarball.sh test-runner.tar

# Only what runs when a student runs their tests. The rest of bin/ is drivers
# for Docker and for this runner's own golden tests in tests/.
# TODO: run.sh calls bin/exercism_parser, which the Dockerfile compiles from
# src/ and include/. It has to be a binary the kernel can execute, so it
# either comes from the sysroot or gets added here once built for it.
CONTENTS=(bin/run.sh)

# Everything lands under here, matching the Docker image's WORKDIR.
# Explicitly not an absolute path: see the `stage` function.
PREFIX="opt/test-runner"

main() {
    # Made absolute now, because tar's --directory would otherwise resolve a
    # relative path inside the staging tree. Not via realpath, which on BSD
    # refuses a path that does not exist yet.
    local output="${1:-test-runner.tar}"
    [[ "${output}" == /* ]] || output="${PWD}/${output}"
    mkdir -p "$(dirname "${output}")"

    local tar
    tar="$(find_gnu_tar)"

    local staging
    staging="$(mktemp -d)"
    # shellcheck disable=SC2064
    trap "rm -rf '${staging}'" EXIT

    stage "${staging}"
    archive "${tar}" "${staging}" "${output}"

    echo "Wrote ${output} ($(wc -c < "${output}" | tr -d ' ') bytes)"
    echo "sha256: $(sha256 "${output}")"
}

# The reproducibility flags below are GNU-only. BSD tar takes different ones and
# writes different headers, so rather than silently produce a tarball that
# hashes differently on a maintainer's Mac than in CI, insist on GNU tar.
find_gnu_tar() {
    local candidate
    for candidate in tar gtar; do
        if command -v "${candidate}" > /dev/null && "${candidate}" --version 2> /dev/null | grep -q "GNU tar"; then
            echo "${candidate}"
            return
        fi
    done

    echo "GNU tar is required (on macOS: brew install gnu-tar)" >&2
    exit 1
}

stage() {
    local staging="$1"

    local entry
    for entry in "${CONTENTS[@]}"; do
        mkdir -p "${staging}/${PREFIX}/$(dirname "${entry}")"
        # -a preserves the executable bits, which bin/run.sh needs.
        cp -a "${entry}" "${staging}/${PREFIX}/${entry}"
    done
}

archive() {
    local tar="$1" staging="$2" output="$3"

    # The archive has to be byte-identical between runs: the published S3 path
    # is derived from its hash, so an unstable tarball would mint a new
    # immutable prefix on every deploy and re-publish the same bytes under a
    # new URL. --sort fixes the entry order, --mtime the timestamps, the
    # ownership flags strip whoever happened to run the build, and ustar has
    # no extension headers that could carry anything else.
    "${tar}" \
        --create \
        --file "${output}" \
        --directory "${staging}" \
        --sort=name \
        --mtime="@0" \
        --owner=0 \
        --group=0 \
        --numeric-owner \
        --format=ustar \
        "${PREFIX%%/*}"
}

sha256() {
    if command -v sha256sum > /dev/null; then
        sha256sum "$1" | cut -d' ' -f1
    else
        shasum -a 256 "$1" | cut -d' ' -f1
    fi
}

main "$@"
