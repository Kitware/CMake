#!/bin/sh

# Report whether this clone and its development container are ready to use
# and, if they are not, print what remains to be set up.  Run each time a
# tool attaches to the container.  See `Help/dev/devcontainer.rst`.

set -u

cd "$(dirname "$0")/.."

# Check that development setup is up-to-date, the way our `pre-commit` hook
# does.  `Utilities/SetupForDevelopment.sh` is interactive, so the container
# cannot run it on the developer's behalf.
eval "$(grep '^SetupForDevelopment_VERSION=' Utilities/SetupForDevelopment.sh)"
setup_done=$(git config --get hooks.SetupForDevelopment || echo 0)
if test "$setup_done" -lt "${SetupForDevelopment_VERSION:-0}"; then
    cat <<MESSAGE
git: this work tree is not set up for development.
Run 'Utilities/SetupForDevelopment.sh' to configure your Git identity and
install the project's commit hooks.  The work tree is shared with the host,
so running it here sets up both.
MESSAGE
else
    echo "git: this work tree is set up for development."
fi
