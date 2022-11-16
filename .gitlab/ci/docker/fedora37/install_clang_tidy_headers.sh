#!/bin/sh

set -e

# Packages for building the clang-tidy plugin.
# TODO: Upstream this as a proper Fedora package.
dnf install --setopt=install_weak_deps=False -y \
    'dnf-command(download)' \
    rpm-build \
    python3-devel \
    clang-tools-extra
clang_source_rpm=$(rpm -q --queryformat '%{SOURCERPM}' clang-tools-extra)
clang_version=$(rpm -q --queryformat '%{VERSION}' clang-tools-extra)
dnf download --source -y clang
rpm -i "$clang_source_rpm"
rpmbuild -bp /root/rpmbuild/SPECS/clang.spec
cd "/root/rpmbuild/BUILD/clang-tools-extra-$clang_version.src"
find clang-tidy -name '*.h' | tar -cf /root/clang-tidy-headers.tar -T -
