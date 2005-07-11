#!/bin/sh

MACHINES=""
MACHINES="${MACHINES} naboo"
MACHINES="${MACHINES} shannara"
MACHINES="${MACHINES} krondor"
MACHINES="${MACHINES} rapture"
MACHINES="${MACHINES} destiny"

run()
{
    CMD="'$1'"; shift; for i in "$@"; do CMD="${CMD} '$i'"; done
    eval "$CMD"
}

clean()
{
    for m in $MACHINES; do
        ./cmake_release.sh remote $m clean
    done
}

binary()
{
    for m in $MACHINES; do
        nohup ./cmake_release.sh remote_binary $m >$m.log 2>&1 &
    done
}

logs()
{
    for m in $MACHINES; do
        nohup xterm -geometry 80x10 -title "$m" -e tail -f $m.log >/dev/null 2>&1 &
    done
}

[ ! -z "$1" ] && run "$@"
