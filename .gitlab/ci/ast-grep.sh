#!/bin/sh

set -e

readonly version="0.45.3"
baseurl="https://github.com/ast-grep/ast-grep/releases/download/$version"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        shatool="sha256sum"
        sha256sum="f8ac830881339d1edee6b2652f54798c0f4da5a827f2db38a08ee31117783ce8"
        filename="app-x86_64-unknown-linux-gnu"
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

echo "$sha256sum  $tarball" > ast-grep.sha256sum
curl -OL "$baseurl/$tarball"
$shatool --check ast-grep.sha256sum
mkdir -p ast-grep
unzip -d ast-grep "$tarball" -x sg
rm "$tarball" ast-grep.sha256sum
