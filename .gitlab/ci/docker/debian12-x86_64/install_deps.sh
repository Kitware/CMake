#!/bin/sh

set -e

apt-get install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

curl -L -O https://github.com/IronLanguages/ironpython3/releases/download/v3.4.0/ironpython_3.4.0.deb
echo '7dcd10b7a0ec0342bd7e20eebb597a96bb15267eb797d59358a3b1cfaa3e1adc  ironpython_3.4.0.deb' > ironpython.sha256sum
sha256sum --check ironpython.sha256sum
dpkg -i ironpython_3.4.0.deb
rm ironpython_3.4.0.deb ironpython.sha256sum

# Perforce
curl -L https://www.perforce.com/downloads/perforce/r21.2/bin.linux26x86_64/helix-core-server.tgz -o - \
  | tar -C /usr/local/bin -xvzf - -- p4 p4d
