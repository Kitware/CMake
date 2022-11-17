#!/bin/sh

set -e

dnf install -y --setopt=install_weak_deps=False \
    gcc-c++ mpfr-devel libmpc-devel isl-devel flex bison file findutils diffutils git-core
dnf clean all
