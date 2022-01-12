#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="PDCurses"
readonly ownership="PDCurses Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmpdcurses"
readonly repo="https://github.com/wmcbrine/PDCurses.git"
readonly tag="f1cd4f4569451a5028ddf3d3c202f0ad6b1ae446"
readonly shortlog=false
readonly paths="
  README.md
  *.h
  common/acs437.h
  common/acsuni.h
  pdcurses/README.md
  pdcurses/*.c
  wincon/README.md
  wincon/*.c
  wincon/*.h
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    echo "* -whitespace" > .gitattributes
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
