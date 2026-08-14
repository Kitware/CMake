#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="libuv"
readonly ownership="libuv upstream <libuv@googlegroups.com>"
readonly subtree="Utilities/cmlibuv"
readonly repo="https://github.com/libuv/libuv.git"
readonly tag="d2a45ce364ed97e916be3bcecfd756f15d852473" # v1.52.1 + fixes
readonly shortlog=false
readonly exact_tree_match=false
readonly paths="
  LICENSE
  include
  src
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    echo "* -whitespace" > .gitattributes
    echo >> src/unix/aix-common.c
    echo >> src/unix/ibmi.c
    sed -i '
        s/UV_VERSION_PATCH 2/UV_VERSION_PATCH 1/
        s/UV_VERSION_IS_RELEASE 0/UV_VERSION_IS_RELEASE 1/
        s/UV_VERSION_SUFFIX "dev"/UV_VERSION_SUFFIX ""/
    ' include/uv/version.h
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
