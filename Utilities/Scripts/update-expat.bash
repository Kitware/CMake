#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="expat"
readonly ownership="Expat Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmexpat"
readonly repo="https://github.com/libexpat/libexpat.git"
readonly tag="R_2_8_5"
readonly shortlog=false
readonly exact_tree_match=false
readonly paths="
  expat/lib/*.h
  expat/lib/*.c
  expat/README.md
  expat/ConfigureChecks.cmake
  expat/expat_config.h.cmake
  expat/COPYING
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    fromdos expat/ConfigureChecks.cmake expat/CMakeLists.txt expat/expat_config.h.cmake
    chmod a-x expat/ConfigureChecks.cmake expat/CMakeLists.txt expat/expat_config.h.cmake
    mv expat/* .
    rmdir expat
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
