#!/bin/sh

set -e

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="98c60ecc259a07a54be6fcc0f55990332f493bfe5dad460c0ba83963f5dcb06f"
        filename="ti_cgt_armllvm_4.0.4.LTS_linux-x64_installer.bin"
        dirname="ti-cgt-armllvm_4.0.4.LTS"
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac
readonly shatool
readonly sha256sum

cd .gitlab

echo "$sha256sum  $filename" > ticlang.sha256sum
curl -OL "https://cmake.org/files/dependencies/internal/$filename"
$shatool --check ticlang.sha256sum
chmod +x "$filename"
"./$filename" --mode unattended --prefix .
mv "$dirname" ticlang
rm -f "$filename" ticlang.sha256sum
