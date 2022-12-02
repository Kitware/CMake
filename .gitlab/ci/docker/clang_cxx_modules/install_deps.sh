#!/bin/sh

set -e

dnf install -y --setopt=install_weak_deps=False \
    gcc-c++ cmake ninja-build
dnf clean all
