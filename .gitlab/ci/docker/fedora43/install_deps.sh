#!/bin/sh

set -e

dnf install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

# Add locales.
localedef --no-archive --inputfile=en_US --charmap=ISO-8859-1 en_US.ISO-8859-1
localedef --no-archive --inputfile=en_US --charmap=UTF-8 en_US.UTF-8

# Remove tests for Python packages
for v in 3.14; do
    find /usr/lib64/python${v}/site-packages -type d -a -name tests -exec rm -rf {} +
done
