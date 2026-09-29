#!/bin/sh

set -e
case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="9ab71dfe9b3c48f91f98b12ac8a888f2aa82630495c4610434cde5de97ffa0a2"
        filename="android-sdk-36-1"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum
readonly filename

cd .gitlab

readonly tarball="$filename.tar.gz"
echo "$sha256sum  $tarball" > android-sdk.sha256sum
curl -OL "https://cmake.org/files/dependencies/internal/android/$tarball"
$shatool --check android-sdk.sha256sum
tar xzf "$tarball"
rm -f "$tarball" android-sdk.sha256sum
mv "$filename" android-sdk
