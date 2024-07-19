#!/usr/bin/env bash

# Note: llpkgc *generates* a parser, thus this script requires npm be available

set -e
set -x
shopt -s dotglob

readonly name="llpkgc"
readonly ownership="llpkgc upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmllpkgc"
readonly repo="https://gitlab.kitware.com/utils/llpkgc.git"
readonly tag="7958a1de42b9eec04676d547f6fcf5daa425fbcc"
readonly shortlog=false
readonly paths="
  bin
  src
  *.json
"

extract_source() {
  git_archive
  npm install
  npm run build
  mv build/llpkgc/* "${extractdir}/${name}-reduced"

  pushd "${extractdir}/${name}-reduced"
  rm CMakeLists.txt *.json
  rm -rf src bin
  echo "* -whitespace" > .gitattributes
  popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
