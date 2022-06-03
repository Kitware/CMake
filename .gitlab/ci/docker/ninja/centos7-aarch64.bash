#!/usr/bin/env bash

set -e
set -x

cleanup() {
    docker container rm -fv "$build_container" >/dev/null 2>&1 || true
    docker image rm -f "$build_image" >/dev/null 2>&1 || true
}

readonly suffix="-$(date -u +%Y-%m-%d)-${RANDOM}"
readonly build_container="ninja-build-linux-aarch64$suffix"
readonly build_image="ninja:build-linux-aarch64$suffix"
readonly git_tag="${1-v1.11.0}"

trap "cleanup" EXIT INT TERM

docker image build --build-arg GIT_TAG="$git_tag" --tag="$build_image" "${BASH_SOURCE%/*}/centos7-aarch64"
docker container create --name "$build_container" "$build_image"
docker cp "$build_container:/ninja/ninja" "ninja"
