#!/bin/sh

set -e

apt-get install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

curl -L -O https://github.com/IronLanguages/ironpython2/releases/download/ipy-2.7.10/ironpython_2.7.10.deb
echo 'e1aceec1d49ffa66e9059a52168a734999dcccc50164a60e2936649cae698f3e  ironpython_2.7.10.deb' > ironpython.sha256sum
sha256sum --check ironpython.sha256sum
dpkg -i ironpython_2.7.10.deb
rm ironpython_2.7.10.deb ironpython.sha256sum

# Perforce
curl -L https://www.perforce.com/downloads/perforce/r21.2/bin.linux26x86_64/helix-core-server.tgz -o - \
  | tar -C /usr/local/bin -xvzf - -- p4 p4d
