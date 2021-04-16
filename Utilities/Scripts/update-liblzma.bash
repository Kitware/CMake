#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="liblzma"
readonly ownership="liblzma upstream <xz-devel@tukaani.org>"
readonly subtree="Utilities/cmliblzma"
readonly repo="https://git.tukaani.org/xz.git"
readonly tag="v5.2.5"
readonly shortlog=false
readonly paths="
  COPYING
  src/common/common_w32res.rc
  src/common/mythread.h
  src/common/sysdefs.h
  src/common/tuklib_common.h
  src/common/tuklib_config.h
  src/common/tuklib_cpucores.c
  src/common/tuklib_cpucores.h
  src/common/tuklib_integer.h
  src/liblzma/
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    mv src/common .
    mv src/liblzma .
    rmdir src
    rm liblzma/Makefile.*
    rm liblzma/*/Makefile.*
    rm liblzma/liblzma.map
    rm liblzma/validate_map.sh
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
