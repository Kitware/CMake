#!/bin/sh

set -e

dnf install \
    --setopt=install_weak_deps=False \
    --setopt=fastestmirror=True \
    --setopt=max_parallel_downloads=10 \
    -y \
    $(grep '^[^#]\+$' /root/rvm_packages.lst)

gpg2 --keyserver hkps://keyserver.ubuntu.com \
     --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 \
                 7D2BAF1CF37B13E2069D6956105BD0E739499BDB

curl -sSL https://get.rvm.io | bash -s stable

# keep version in sync with `env_fedora*_makefiles.cmake`
/usr/local/rvm/bin/rvm install ruby-3.0.4

for p in archives examples gem-cache log src; do
    touch /usr/local/rvm/${p}/.tar_exclude
done

tar -C /usr/local --exclude-tag-under=.tar_exclude -cf /root/rvm.tar rvm
