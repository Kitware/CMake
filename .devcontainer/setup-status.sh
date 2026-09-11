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
fi

readonly host="${GITLAB_HOST:-gitlab.com}"

if glab auth status --hostname "$host" > /dev/null 2>&1; then
    exit 0
fi

# The OAuth flows need the application ID of an OAuth application registered
# on the instance.  Offer them only when one is available, and prefer the
# device flow because it needs no redirect back into the container.
if test -n "${GITLAB_CLIENT_ID:-}"; then
    readonly login="glab auth login --hostname $host --device"
    readonly hint=""
else
    readonly login="glab auth login --hostname $host"
    readonly hint="
Setting GITLAB_CLIENT_ID, on the host, to the application ID of a GitLab
OAuth application offers to sign in through that application instead.
"
fi

cat <<MESSAGE
glab: no credential for $host yet.
Set one up in either of two ways:

  * Run this in the container, which stores the credential under
    ~/.config/glab-cli, on a volume that persists across rebuilds:

      $login

  * Or set GITLAB_TOKEN, on the host, to a GitLab personal access token
    before starting the container.  It is passed through automatically.
$hint
Run '.devcontainer/setup-status.sh' here in the container to check again.
MESSAGE
