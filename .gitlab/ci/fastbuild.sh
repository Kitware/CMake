#!/bin/sh

set -e

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="2c7b4c833c7f2846119aaa72bfa92df5b7da1aa17a0e62187ac0bbcbbf5cce8e"
        filename="FASTBuild-Linux-x64-v1.15"
        exenames="fastbuild/fbuild fastbuild/fbuildworker"
        ;;
    Darwin-*)
        shatool="shasum -a 256"
        sha256sum="875fedc71a2b479b22e90fcc77db75513b1d88794fd71cfe3889f41b225efbaa"
        filename="FASTBuild-OSX-x64+ARM-v1.15"
        exenames="fastbuild/FBuild fastbuild/FBuildWorker"
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

echo "$sha256sum  $tarball" > fastbuild.sha256sum
# This URL is only visible inside of Kitware's network.  See above filename table.
curl -OL "https://cmake.org/files/dependencies/internal/$tarball"
$shatool --check fastbuild.sha256sum
mkdir -p fastbuild
unzip -d fastbuild -q "$tarball"
chmod +x $exenames
rm "$tarball" fastbuild.sha256sum
