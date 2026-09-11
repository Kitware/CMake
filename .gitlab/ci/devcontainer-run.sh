#!/bin/sh

set -e

# This job runs in a container whose filesystem is `overlayfs`, over which
# `podman` cannot stack its own `overlay` storage driver:
#
#     'overlay' is not supported over overlayfs, a mount_program is required
#
# `vfs` copies each layer rather than stacking it, which is slower but asks
# nothing of the filesystem underneath.  Name it in the environment so that
# every `podman` command below addresses the same storage, the build included.
export STORAGE_DRIVER=vfs

readonly dockerfile=".devcontainer/Dockerfile"
readonly name="cmake-dev-container"

# Build the container image from the devcontainer Dockerfile
echo "# Building devcontainer image"
podman build -t "$name" -f "$dockerfile" .devcontainer

# Ensure named volumes exist for ccache and glab-cli cache persistence
echo "# Ensuring volumes exist"
podman volume create cmake-dev-ccache 2>/dev/null || true
podman volume create cmake-dev-glab-cli 2>/dev/null || true

# Run the container with volumes mounted.
#
# The job's own container has no cgroup controllers delegated to it, so
# `podman` cannot place the container it starts under one:
#
#     crun: controller `pids` is not available under /sys/fs/cgroup/...
#
# Ask for no cgroup at all.  Verification needs no resource limits, and there
# is nothing to limit them with.  There is no terminal either, so do not ask
# for one.
echo "# Starting devcontainer"
podman run --rm \
    --cgroups=disabled \
    -v "$PWD:/home/cmake-dev/workspace:Z" \
    -v cmake-dev-ccache:/home/cmake-dev/.cache/ccache:Z \
    -v cmake-dev-glab-cli:/home/cmake-dev/.config/glab-cli:Z \
    --workdir /home/cmake-dev/workspace \
    "$name" \
    .gitlab/ci/devcontainer-verify.sh
