#!/bin/sh

set -e

dnf install \
    --setopt=install_weak_deps=False \
    --setopt=fastestmirror=True \
    --setopt=max_parallel_downloads=10 \
    -y \
    $(grep '^[^#]\+$' /root/rust_packages.lst)

typos_version=1.29.4
cargo install --root /usr/local --version "$typos_version" typos-cli

tar -C /usr/local -cf /root/rust.tar bin/typos
