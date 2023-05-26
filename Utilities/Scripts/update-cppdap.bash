#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="cppdap"
readonly ownership="cppdap Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmcppdap"
readonly repo="https://github.com/google/cppdap.git"
readonly tag="03cc18678ed2ed8b2424ec99dee7e4655d876db5" # 2023-05-25
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
    fromdos LICENSE include/dap/* src/*
    echo "" >> LICENSE
    echo "" >> src/nlohmann_json_serializer.h
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
