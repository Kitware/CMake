#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="jsoncpp"
readonly ownership="JsonCpp Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmjsoncpp"
readonly repo="https://github.com/open-source-parsers/jsoncpp.git"
readonly tag="42e892d96e47b1f6e29844cc705e148ec4856448"
readonly shortlog=false
readonly paths="
  LICENSE
  include/json
  src/lib_json
"
readonly remove="
  include/json/autolink.h
  src/lib_json/CMakeLists.txt
  src/lib_json/sconscript
  src/lib_json/version.h.in
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    rm $remove
    echo "* -whitespace" > .gitattributes
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
