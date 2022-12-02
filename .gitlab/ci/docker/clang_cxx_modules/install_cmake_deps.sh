#!/bin/sh

set -e

dnf install -y --setopt=install_weak_deps=False \
    file git-core
dnf clean all
