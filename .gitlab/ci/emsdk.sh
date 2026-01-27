#!/bin/sh

set -e

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)
        ;;
    *)
        echo "Unrecognized platform $(uname -s)-$(uname -m)"
        exit 1
        ;;
esac

cd .gitlab

version=4.0.9
dirname="emsdk-$version"
filename="$dirname.tar.gz"
curl -OJL "https://github.com/emscripten-core/emsdk/archive/refs/tags/$version.tar.gz"
tar xzf "$filename"
mv "$dirname" emsdk
emsdk/emsdk install "$version"
emsdk/emsdk activate "$version"

rm -f "$filename"
