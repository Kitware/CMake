#!/bin/sh

set -e

dnf install \
    --setopt=install_weak_deps=False \
    --setopt=fastestmirror=True \
    --setopt=max_parallel_downloads=10 \
    -y \
    $(grep '^[^#]\+$' /root/deps_packages.lst)

# Fedora no longer packages python2 numpy.
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py -o - | python2
pip2.7 install --disable-pip-version-check --no-input --no-compile --cache-dir /var/cache/pip numpy

# Remove demos and Python2 tests
for p in Demo test; do
    rm -rf /usr/lib64/python2.7/${p}
done

# Remove tests for numpy
for v in 2.7 3.12; do
    find /usr/lib64/python${v}/site-packages/numpy -type d -a -name tests -exec rm -rf {} +
done

# Remove some other packages tests
find /usr/lib64/python3.12/site-packages/breezy -type d -a -name tests -exec rm -rf {} +

# Perforce
curl -L https://www.perforce.com/downloads/perforce/r21.2/bin.linux26x86_64/helix-core-server.tgz -o - \
  | tar -C /usr/local/bin -xvzf - -- p4 p4d
