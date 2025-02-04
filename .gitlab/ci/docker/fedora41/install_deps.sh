#!/bin/sh

set -e

dnf install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

# Remove tests for Python packages
for v in 3.13; do
    find /usr/lib64/python${v}/site-packages -type d -a -name tests -exec rm -rf {} +
done
