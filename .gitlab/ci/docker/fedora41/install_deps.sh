#!/bin/sh

set -e

dnf install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

# Remove tests for numpy
for v in 3.13; do
    find /usr/lib64/python${v}/site-packages/numpy -type d -a -name tests -exec rm -rf {} +
done

# Remove some other packages tests
find /usr/lib64/python3.13/site-packages/breezy -type d -a -name tests -exec rm -rf {} +

# Ruby rbenv
rbenv install 3.1.2
rbenv global 3.1.2

# Perforce
curl -L https://www.perforce.com/downloads/perforce/r21.2/bin.linux26x86_64/helix-core-server.tgz -o - \
  | tar -C /usr/local/bin -xvzf - -- p4 p4d
