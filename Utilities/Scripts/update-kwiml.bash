#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="KWIML"
readonly ownership="KWIML Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/KWIML"
readonly repo="https://gitlab.kitware.com/utils/kwiml.git"
readonly tag="master" # When updating, sync KWIML_VERSION below!
readonly shortlog=true
readonly exact_tree_match=false
readonly paths="
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    cp src/version.h.in include/kwiml/version.h
    sed -i 's/@KWIML_VERSION@/1.0.0/;s/@KWIML_VERSION_DECIMAL@/1000000/' include/kwiml/version.h
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
