#!/bin/sh

set -e

if test "$CMAKE_CI_JOB_NIGHTLY_NINJA" = "true" -a "$CMAKE_CI_NIGHTLY" = "true"; then
    exec .gitlab/ci/ninja-nightly.sh
fi

readonly version="1.11.0"
baseurl="https://github.com/ninja-build/ninja/releases/download/v$version"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="9726e730d5b8599f82654dc80265e64a10a8a817552c34153361ed0c017f9f02"
        filename="ninja-linux"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="b002eb77cfcef6d329cccf8b1cc7ad138302d6e19b5b76b10b4c4d38564b47b5"
        # Use binary built by adjacent 'docker/ninja/centos7-aarch64.bash' script.
        baseurl="https://cmake.org/files/dependencies"
        filename="ninja-$version-1-linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="21915277db59756bfc61f6f281c1f5e3897760b63776fd3d360f77dd7364137f"
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
