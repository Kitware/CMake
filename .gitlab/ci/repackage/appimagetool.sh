#!/usr/bin/env bash

set -e

arch="$1"
version="${2-1.9.0.20250814}"

dir="appimagetool-$version-$arch"
mkdir "$dir"
mkdir -p "$dir/lib/appimagetool"

filename="appimagetool-$arch.AppImage"
curl -OL "https://github.com/AppImage/appimagetool/releases/download/continuous/$filename"
chmod +x "$filename"
"./$filename" --appimage-extract
mv "squashfs-root/usr/bin" "$dir/bin"
rm -rf "$filename" "squashfs-root"

filename="runtime-$arch"
curl -OL "https://github.com/AppImage/type2-runtime/releases/download/continuous/$filename"
mv "$filename" "$dir/lib/appimagetool/runtime"

cat >"$dir/README.txt" <<EOF
This was packaged using CMake's ".gitlab/ci/repackage/appimagetool.sh" script.
EOF

tar czf "$dir.tar.gz" "$dir"
rm -rf "$dir"
