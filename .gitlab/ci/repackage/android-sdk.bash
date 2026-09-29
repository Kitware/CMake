#!/usr/bin/env bash

set -e

cleanup() {
    if test -d "$tmpdir"; then
        rm -rf "$tmpdir"
    fi
}

trap "cleanup" EXIT

readonly version="${1-36.0.0}"
readonly revision="${2-1}"
readonly version_major="${version%%.*}"
readonly android_home_name="android-sdk-${version_major}-${revision}"
readonly android_home_archive="$PWD/${android_home_name}.tar.gz"

readonly tmpdir="$(mktemp -d)"

readonly android="$tmpdir/android"
curl -L "https://dl.google.com/android/cli/latest/linux_x86_64/android" -o "$android"
chmod +x "$android"

export HOME="$tmpdir"
export ANDROID_HOME="$tmpdir/$android_home_name"
export ANDROID_USER_HOME="$tmpdir/.android"
ANDROID_CLI_FRESH_INSTALL=1 "$android"
"$android" sdk install "build-tools/${version}" "platforms/android-${version_major}"

echo >"${ANDROID_HOME}/README.md" "This was repackaged from an installation by

    android sdk install build-tools/${version} platforms/android-${version_major}

using CMake's \".gitlab/ci/repackage/android-sdk.bash\" script."

tar -C "$tmpdir" -czf "${android_home_archive}" \
    "${android_home_name}/build-tools" \
    "${android_home_name}/platforms" \
    "${android_home_name}/README.md"
echo "Repackaged Android SDK as \"${android_home_archive}\""
