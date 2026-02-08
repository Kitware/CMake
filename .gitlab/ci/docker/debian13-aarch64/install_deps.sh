#!/bin/sh

set -e

apt-get install -y $(grep '^[^#]\+$' /root/deps_packages.lst)

# Add locales.
sed -i -E '/^# en_US[ .](ISO-8859-1|UTF-8)( |$)/ s/^# //' /etc/locale.gen
dpkg-reconfigure --frontend=noninteractive locales
