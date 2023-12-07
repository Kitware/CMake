#!/bin/sh

set -e

apt-get install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

curl -L -O https://github.com/IronLanguages/ironpython2/releases/download/ipy-2.7.12/ironpython_2.7.12.deb
echo 'b7b90c82cf311dd3faf290ce3f274af5128b96db884a88dd643ce141bbf12fb9  ironpython_2.7.12.deb' > ironpython.sha256sum
sha256sum --check ironpython.sha256sum
dpkg -i ironpython_2.7.12.deb
rm ironpython_2.7.12.deb ironpython.sha256sum
