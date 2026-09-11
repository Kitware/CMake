#!/bin/sh

# Add the Kitware APT repository, which carries CMake releases newer than the
# ones the distribution provides.  See `Help/dev/devcontainer.rst`.

set -e

readonly key_sha256="$1"

if test -z "$key_sha256"; then
    echo "usage: $0 <sha256-of-kitware-archive-key>" >&2
    exit 1
fi

# Install without asking questions.
export DEBIAN_FRONTEND=noninteractive

# `VERSION_CODENAME` names the suite the repository provides for the
# distribution the container is based on.
. /etc/os-release

readonly sources=/etc/apt/sources.list.d/kitware.sources
readonly keyring=/usr/share/keyrings/kitware-archive-keyring
readonly key_url=https://apt.kitware.com/keys/kitware-archive-latest.asc

# Describe the repository, verified with the keyring named as the argument.
write_sources() {
    cat > "$sources" <<EOF
Types: deb
URIs: https://apt.kitware.com/ubuntu/
Suites: ${VERSION_CODENAME}
Components: main
Signed-By: $1
EOF
}

apt-get update

# The base image carries neither `curl` nor a certificate store, and neither
# the key nor the repository can be reached without one: both redirect HTTP
# to HTTPS.
apt-get install -y ca-certificates curl

# Trust the repository with the key it publishes, checked against the hash our
# caller pins, just long enough to install `kitware-archive-keyring`.  Once
# that package provides the key, `apt` follows the rotations Kitware makes to
# it each year, which a pinned hash would not.  `apt` reads an armored key
# only from a `.asc` file, and the package provides a `.gpg` one, so name the
# file each step uses accordingly.
curl -fsSL -o "$keyring.asc" "$key_url"
echo "$key_sha256  $keyring.asc" | sha256sum --check
write_sources "$keyring.asc"
apt-get update
apt-get install -y kitware-archive-keyring
write_sources "$keyring.gpg"
rm "$keyring.asc"
