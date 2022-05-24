#!/bin/sh

set -e

if test "$CMAKE_CI_JOB_NIGHTLY_NINJA" = "true" -a "$CMAKE_CI_NIGHTLY" = "true"; then
    exec .gitlab/ci/ninja-nightly.sh
fi

readonly version="1.10.2"
baseurl="https://github.com/ninja-build/ninja/releases/download/v$version"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="763464859c7ef2ea3a0a10f4df40d2025d3bb9438fcb1228404640410c0ec22d"
        filename="ninja-linux"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="c0c29925fd7f0f24981b3b9d18353c7111c9af59eb6e6b0ffc0c4914cdc7999c"
        # Use binary built by adjacent 'docker/ninja/centos7-aarch64.bash' script.
        baseurl="https://cmake.org/files/dependencies"
        filename="ninja-$version-1-linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="6fa359f491fac7e5185273c6421a000eea6a2f0febf0ac03ac900bd4d80ed2a5"
        filename="ninja-mac"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

readonly tarball="$filename.zip"

cd .gitlab

echo "$sha256sum  $tarball" > ninja.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check ninja.sha256sum
./cmake/bin/cmake -E tar xf "$tarball"
