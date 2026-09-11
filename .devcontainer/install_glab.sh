#!/bin/sh

# Install the GitLab CLI, `glab`, and `glab-axi`, an agent-ergonomic wrapper
# around it.  Neither is provided by the distribution.
# See `Help/dev/devcontainer.rst`.

set -e

readonly glab_version="1.114.0"
readonly glab_axi_version="0.6.0"

case "$(uname -m)" in
    x86_64)
        arch="amd64"
        sha256sum="00e892a80d586a1e8b8fdc035321923db99dce0caa3b0c4fd72c5337ffdb1c48"
        ;;
    aarch64)
        arch="arm64"
        sha256sum="d34d7ddb96ce5e5f3423d7e8053cb14c36bd93984e4b96320f7e20a341b83498"
        ;;
    *)
        echo "Unsupported architecture: $(uname -m)" >&2
        exit 1
        ;;
esac

readonly filename="glab_${glab_version}_linux_${arch}.tar.gz"
readonly baseurl="https://gitlab.com/gitlab-org/cli/-/releases/v${glab_version}/downloads"

cd /tmp
curl -L -o "$filename" "$baseurl/$filename"
echo "$sha256sum  $filename" > glab.sha256sum
sha256sum --check glab.sha256sum
tar -C /usr/local -xzf "$filename" bin/glab
rm "$filename" glab.sha256sum

# Enable shell completion for interactive use.
mkdir -p /etc/bash_completion.d
/usr/local/bin/glab completion --shell bash > /etc/bash_completion.d/glab

# `glab-axi` is distributed only through npm.  Its command surface is
# documented at https://axi.md.
npm install --global --no-audit --no-fund "glab-axi@${glab_axi_version}"
