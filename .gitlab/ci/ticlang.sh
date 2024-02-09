#!/bin/sh

set -e

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="c69ac58e403b82eac1c407cc67b35fab5d95c5d8db75b019095f9412aacff27d"
        filename="ti_cgt_armllvm_3.2.1.LTS_linux-x64_installer.bin"
        dirname="ti-cgt-armllvm_3.2.1.LTS"
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
