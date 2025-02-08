#!/bin/sh

set -e

result=0

echo "Running 'typos' on source code..."
typos || result=1

# FIXME(typos): checking commit messages hits false positives
# on "words" inside commit hashes.  We'd need a way to disable
# checking of combined identifiers to avoid this.

exit $result
