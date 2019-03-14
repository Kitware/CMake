#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="zstd"
readonly ownership="zstd upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmzstd"
readonly repo="https://github.com/facebook/zstd.git"
readonly tag="v1.3.8"
readonly shortlog=false
readonly paths="
  LICENSE
  README.md
  lib/common/*.c
  lib/common/*.h
  lib/compress/*.c
  lib/compress/*.h
  lib/decompress/*.c
  lib/decompress/*.h
  lib/deprecated/*.c
  lib/deprecated/*.h
  lib/dictBuilder/*.c
  lib/dictBuilder/*.h
  lib/zstd.h
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    echo "* -whitespace" > .gitattributes
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
