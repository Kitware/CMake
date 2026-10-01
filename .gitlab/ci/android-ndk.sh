#!/bin/sh

set -e
case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="753611f410d002cfcd3f3dc2ef49aad532089d3180b436c060a90bf0fcb64df2"
        dirname="android-ndk-r30"
        platform="linux"
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

readonly archive="$dirname-$platform.zip"
echo "$sha256sum  $archive" > android-ndk.sha256sum
curl -OL "https://cmake.org/files/dependencies/internal/android/$archive"
$shatool --check android-ndk.sha256sum
unzip -q "$archive"
rm -f "$archive" android-ndk.sha256sum
mv "$dirname" android-ndk
