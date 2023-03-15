#!/bin/bash

set -e

quietly() {
  readonly log="/tmp/quietly-$RANDOM.log"
  if ! "$@" >"$log" 2>&1; then
    ret=$?
    cat "$log"
    rm -f "$log"
    exit $ret
  fi
  rm -f "$log"
}

if test -r ".gitlab/ci/pre_build_${CMAKE_CONFIGURATION}.sh"; then
  source ".gitlab/ci/pre_build_${CMAKE_CONFIGURATION}.sh"
fi
