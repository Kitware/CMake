#!/bin/sh

set -e

# clang-tidy headers
apt-get install -y \
    gnupg2
GNUPGHOME=$(mktemp -d)
export GNUPGHOME
keyid=6084F3CF814B57C1CF12EFD515CF4D18AF4F7421
gpg2 --keyserver hkps://keyserver.ubuntu.com --recv-keys "$keyid"
gpg2 -o /usr/share/keyrings/llvm.gpg --export "$keyid"
rm -rf "$GNUPGHOME"
unset GNUPGHOME
echo 'deb [signed-by=/usr/share/keyrings/llvm.gpg] http://apt.llvm.org/buster/ llvm-toolchain-buster-14 main' > /etc/apt/sources.list.d/llvm.list
apt-get update
apt-get install -y \
    clang-tidy-14 \
    libclang-14-dev

apt-get clean
