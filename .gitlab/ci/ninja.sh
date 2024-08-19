#!/bin/sh

set -e

if test "$CMAKE_CI_JOB_NIGHTLY_NINJA" = "true" -a "$CMAKE_CI_NIGHTLY" = "true"; then
    exec .gitlab/ci/ninja-nightly.sh
fi

readonly version="1.12.1"
baseurl="https://github.com/ninja-build/ninja/releases/download/v$version"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="6f98805688d19672bd699fbbfa2c2cf0fc054ac3df1f0e6a47664d963d530255"
        filename="ninja-linux"
        ;;
    Linux-aarch64)
        shatool="sha256sum"
        sha256sum="5c25c6570b0155e95fce5918cb95f1ad9870df5768653afe128db822301a05a1"
        filename="ninja-linux-aarch64"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="89a287444b5b3e98f88a945afa50ce937b8ffd1dcc59c555ad9b1baf855298c9"
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
