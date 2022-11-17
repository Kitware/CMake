#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="libuv"
readonly ownership="libuv upstream <libuv@googlegroups.com>"
readonly subtree="Utilities/cmlibuv"
readonly repo="https://github.com/libuv/libuv.git"
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
