#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="nghttp2"
readonly ownership="nghttp2 upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmnghttp2"
readonly repo="https://github.com/nghttp2/nghttp2.git"
readonly tag="v1.40.0"
readonly shortlog=false
readonly paths="
  COPYING
  lib/*.c
  lib/*.h
  lib/includes/nghttp2/nghttp2.h
  lib/includes/nghttp2/nghttp2ver.h.in
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    echo "* -whitespace" > .gitattributes
    mv lib/includes/nghttp2/nghttp2ver.h.in lib/includes/nghttp2/nghttp2ver.h
    sed -i 's/@PACKAGE_VERSION@/1.40.0/;s/@PACKAGE_VERSION_NUM@/0x012800/' lib/includes/nghttp2/nghttp2ver.h
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
