#!/bin/sh

set -e

apt-get update
apt-get install -y \
    curl \
    gnupg2 \
    procps

gpg2 --keyserver hkps://keyserver.ubuntu.com \
     --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 \
                 7D2BAF1CF37B13E2069D6956105BD0E739499BDB

curl -sSL https://get.rvm.io | bash -s stable

/usr/local/rvm/bin/rvm install ruby-2.7.0

tar -C /usr/local -cf /root/rvm.tar rvm
