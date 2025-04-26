#!/bin/sh

set -e

echo "gem: --no-document" > ~/.gemrc

# Ruby rbenv
export RUBY_CONFIGURE_OPTS=--disable-install-doc
export RUBY_BUILD_CURL_OPTS=-C-
rbenv install 3.4.3 -k -s -v

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
tar -cf /root/rbenv.tar --exclude-from=/tmp/exclude.lst ${RBENV_ROOT}
