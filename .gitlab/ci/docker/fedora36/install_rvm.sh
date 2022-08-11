#!/bin/sh

set -e

gpg2 --keyserver hkps://keyserver.ubuntu.com \
     --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 \
                 7D2BAF1CF37B13E2069D6956105BD0E739499BDB

dnf install --setopt=install_weak_deps=False -y \
    findutils \
    procps \
    which

curl -sSL https://get.rvm.io | bash -s stable

# keep version in sync with `env_fedora*_makefiles.cmake`
/usr/local/rvm/bin/rvm install ruby-3.0.4

tar -C /usr/local -cf /root/rvm.tar rvm
