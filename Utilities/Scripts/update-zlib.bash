#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="zlib"
readonly ownership="zlib upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmzlib"
readonly repo="https://github.com/madler/zlib.git"
readonly tag="v1.2.3"
readonly shortlog=false
readonly paths="
  README

  adler32.c
  compress.c
  crc32.c
  crc32.h
  deflate.c
  deflate.h
  gzio.c
  inffast.c
  inffast.h
  inffixed.h
  inflate.c
  inflate.h
  inftrees.c
  inftrees.h
  trees.c
  trees.h
  uncompr.c
  zconf.h
  zlib.h
  zutil.c
  zutil.h
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    echo "* -whitespace" > .gitattributes
    echo -n "'zlib' general purpose compression library
version 1.2.3, July 18th, 2005

Copyright " > Copyright.txt
    sed -n '/^ (C) 1995-/,+19 {s/^  \?//;s/2004/2005/;p}' README >> Copyright.txt
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
