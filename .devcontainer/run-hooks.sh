#!/bin/sh

# Run the optional local customization hook for one phase of the development
# container's life, named as the sole argument, if the developer has written
# one.  See `Help/dev/devcontainer.rst`.
#
# The hooks live beside this script, in a directory Git ignores in its
# entirety, so customizations never appear in a commit and survive updates to
# the tracked container definition.

set -eu

readonly phase="$1"
readonly devcontainer_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
readonly hooks_dir="$devcontainer_dir/hooks"
readonly hook="$hooks_dir/$phase.sh"

test -f "$hook" || exit 0

# Tell the hook where its own directory is, so that a hook needing a file it
# brought along need not work out where it was installed.
CMAKE_DEVCONTAINER_HOOKS_DIR="$hooks_dir"
export CMAKE_DEVCONTAINER_HOOKS_DIR

# A failed `build` hook fails the image build: the image must be reproducible,
# and a customization that did not apply would leave it quietly wrong.  Every
# other phase runs against a container that already exists, where the same
# strictness would turn a typo in a personal hook into an environment its
# author can no longer open in order to fix it.  Report and carry on instead.
if test "$phase" = build; then
    # The build sees this directory through a read-only bind mount, and keeps
    # nothing a later phase could read back: whatever this hook writes it
    # writes into the image.  So there is no state directory to offer it.
    exec sh -e "$hook"
fi

# Every other phase runs against the bind-mounted source tree, where a hook
# may keep state that outlives the container.  It sits beside the hooks rather
# than among them: the hooks are written by hand and worth carrying to another
# clone, while this is written by whatever they start and worth carrying
# nowhere.  `.dockerignore` also leaves it out of the build context, which a
# hook writing here as `root` would otherwise make unreadable to the build.
CMAKE_DEVCONTAINER_STATE_DIR="$devcontainer_dir/state"
export CMAKE_DEVCONTAINER_STATE_DIR
mkdir -p "$CMAKE_DEVCONTAINER_STATE_DIR"

sh -e "$hook" || echo "run-hooks.sh: $phase hook failed; continuing" >&2
