#!/bin/sh

# Install `ast-grep`, which checks our C++ code against the rules under
# `Utilities/ast-grep`.  It is not provided by the distribution.
# See `Help/dev/devcontainer.rst`.

set -e

# Install the release our CI job does, from `.gitlab/ci/ast-grep.sh`, so that
# a scan here reports what that job reports.
readonly version="0.45.3"

case "$(uname -m)" in
    x86_64)
        sha256sum="f8ac830881339d1edee6b2652f54798c0f4da5a827f2db38a08ee31117783ce8"
        filename="app-x86_64-unknown-linux-gnu.zip"
        ;;
    aarch64)
        sha256sum="b39cfbc58da4b869a88b8a4bc57bd5deb0d24541e704cf7c257da7b53ec81c8f"
        filename="app-aarch64-unknown-linux-gnu.zip"
        ;;
    *)
        echo "Unsupported architecture: $(uname -m)" >&2
        exit 1
        ;;
esac

readonly baseurl="https://github.com/ast-grep/ast-grep/releases/download/${version}"

# The release also carries `sg`, a shorter alias for `ast-grep`.  Leave it
# out, as our CI job does: the name is taken by the `sg` command that runs a
# command under another group.
cd /tmp
curl -L -o "$filename" "$baseurl/$filename"
echo "$sha256sum  $filename" > ast-grep.sha256sum
sha256sum --check ast-grep.sha256sum
unzip -d /usr/local/bin "$filename" ast-grep
rm "$filename" ast-grep.sha256sum

# Enable shell completion for interactive use.
mkdir -p /etc/bash_completion.d
/usr/local/bin/ast-grep completions bash > /etc/bash_completion.d/ast-grep
