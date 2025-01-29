#!/bin/sh

set -e

typos_version=1.29.4
cargo install --root /usr/local --version "$typos_version" typos-cli

strip /usr/local/bin/typos

tar -C /usr/local -cf /root/rust.tar bin/typos
