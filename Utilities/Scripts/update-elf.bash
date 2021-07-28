#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="elf"
readonly ownership="FreeBSD Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmelf"
readonly repo="https://github.com/freebsd/freebsd-src.git"
readonly tag="main"
readonly shortlog=false
readonly paths="
  sys/sys/elf32.h
  sys/sys/elf64.h
  sys/sys/elf_common.h
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    echo "* -whitespace" > .gitattributes
    mv sys/sys/* .
    sed -i -e 's/<sys\/elf_common.h>/"elf_common.h"/g' -e 's/u_int32_t/uint32_t/g' *.h
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
