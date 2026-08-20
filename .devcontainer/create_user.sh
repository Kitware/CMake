#!/bin/sh

# Create the unprivileged user that the container runs as, given its name,
# uid, and gid.  See `Help/dev/devcontainer.rst`.

set -e

readonly username="$1"
readonly uid="$2"
readonly gid="$3"

# The base image already ships an unprivileged user, which usually occupies
# the uid we want.  Remove whoever holds it, and the group holding our gid,
# before creating ours.
if getent passwd "$uid" > /dev/null; then
    userdel --remove "$(getent passwd "$uid" | cut -d: -f1)"
fi
if getent group "$gid" > /dev/null; then
    groupdel "$(getent group "$gid" | cut -d: -f1)"
fi

groupadd --gid "$gid" "$username"
useradd --uid "$uid" --gid "$gid" --create-home --shell /bin/bash "$username"

# Let the user administer the container, e.g. to install more packages.
echo "$username ALL=(ALL) NOPASSWD:ALL" > "/etc/sudoers.d/$username"
chmod 0440 "/etc/sudoers.d/$username"

# Pre-create the directories that `devcontainer.json` mounts volumes over so
# that the volumes inherit the ownership recorded here.  Name the parents too:
# `install -d` records the ownership only of the directories it is given, and
# tools that write elsewhere under them need to own them as well.
install -d -o "$username" -g "$username" \
    "/home/$username/.cache" \
    "/home/$username/.cache/ccache" \
    "/home/$username/.config" \
    "/home/$username/.config/glab-cli" \
    "/home/$username/workspace"
