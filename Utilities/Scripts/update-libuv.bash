#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="libuv"
readonly ownership="libuv upstream <libuv@googlegroups.com>"
readonly subtree="Utilities/cmlibuv"
readonly repo="https://github.com/libuv/libuv.git"
# We cannot import libuv 1.45 or higher because it has higher
# minimum system requirements than we do:
# - It requires C11 atomics from GCC 4.9+.  We support GCC 4.8.
# - It requires Windows 8, we support Windows 7.
readonly tag="v1.44.2"
readonly shortlog=false
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
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
