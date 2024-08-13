#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="cppdap"
readonly ownership="cppdap Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmcppdap"
readonly repo="https://github.com/google/cppdap.git"
readonly tag="c69444ed76f7468b232ac4f989cb8f2bdc100185" # 2024-08-02
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
    echo "" >> src/content_stream.cpp
    echo "" >> src/nlohmann_json_serializer.h
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
