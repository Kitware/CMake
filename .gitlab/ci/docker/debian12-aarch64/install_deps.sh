#!/bin/sh

set -e

apt-get install -y $(grep '^[^#]\+$' /root/deps_packages.lst)
