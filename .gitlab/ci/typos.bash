#!/bin/sh

set -e

result=0

echo "Running 'typos' on source code..."
typos || result=1

if [ -n "$CI_MERGE_REQUEST_DIFF_BASE_SHA" ]; then
  for COMMIT in $(git rev-list "^$CI_MERGE_REQUEST_DIFF_BASE_SHA" "$CI_COMMIT_SHA"); do
    echo "Running 'typos' on commit message of $COMMIT..."
    git show --format=%B -s "$COMMIT" | typos - || result=1
  done
fi

exit $result
