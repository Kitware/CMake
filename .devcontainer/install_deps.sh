#!/bin/sh

# Install the packages listed in `deps_packages.lst` and `dev_packages.lst`.
# See `Help/dev/devcontainer.rst`.

set -e

# Install without asking questions, e.g. the time zone `tzdata` wants.
export DEBIAN_FRONTEND=noninteractive

# Unlike our CI images, keep the documentation that packages carry: the base
# image excludes man pages and the like, which is not what one wants in a
# development environment.  Packages already installed in the base image
# remain without their documentation; `unminimize` restores that.
rm -f /etc/dpkg/dpkg.cfg.d/excludes

apt-get update
apt-get install -y $(grep -h '^[^#]\+$' /root/deps_packages.lst /root/dev_packages.lst)

# Add locales, the way our CI images do, for the tests that need them.
sed -i -E '/^# en_US[ .](ISO-8859-1|UTF-8)( |$)/ s/^# //' /etc/locale.gen
dpkg-reconfigure --frontend=noninteractive locales

# `Utilities/Scripts/clang-format.bash` finds `clang-format-18` by name, but
# make the unversioned name resolve to version 18 as well so that tools
# looking for it get the version our style rules require.
ln -s "$(command -v clang-format-18)" /usr/local/bin/clang-format
