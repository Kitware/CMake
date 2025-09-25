#!/bin/sh

set -e

echo "gem: --no-document" > ~/.gemrc

gpg2 --keyserver hkps://keyserver.ubuntu.com \
     --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 \
                 7D2BAF1CF37B13E2069D6956105BD0E739499BDB

curl -sSL https://get.rvm.io | bash -s stable --ignore-dotfiles

export rvm_silence_banner=1

# keep version in sync with `env_fedora*_makefiles.cmake`
/usr/local/rvm/bin/rvm install ruby-3.3.8 --no-docs --disable-binary

for p in archives docs examples gem-cache log src; do
    touch /usr/local/rvm/${p}/.tar_exclude
done

cat <<EOF >/tmp/exclude.lst
*LICENSE*
*/doc/*
*/man/*
*.md
BSDL
CONTRIBUTING.*
COPYING
LEGAL
PSFL
README.rdoc
History.rdoc
gem_make.out
test-unit-*/test
rss-*/test
EOF
tar -C /usr/local \
    --exclude-tag-under=.tar_exclude \
    --exclude-from=/tmp/exclude.lst \
    -cf /root/rvm.tar rvm
