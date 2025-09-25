#!/bin/sh

set -e

die () {
    echo >&2 "$@"
    exit 1
}

test -x "$( which tmux 2>/dev/null )" || die "\`tmux\` is required"
test -n "$TMUX" || die "must be running within a \`tmux\` session"
test -x "$( which watch )" || die "\`watch\` is required"

editor="${EDITOR:-nano}"
readonly editor

test -x "$( which "$EDITOR" )" || die "\`$editor\` is required"

tmux split-window -v -l 10 -c "$PWD" 'watch --interval 1 cmake -P match.cmake'
tmux select-pane -l
case "$editor" in
    *vi*)
        "$editor" re.txt content.txt -c 'vsp|bn'
        ;;
    *)
        "$editor" re.txt content.txt
        ;;
esac
