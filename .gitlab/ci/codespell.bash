#!/bin/sh

set -e

result=0

# 'codespell' with no arguments adds a leading './' to all paths.
# Avoid that by globbing top-level entries explicitly.
shopt -s dotglob
echo "Running 'codespell' on source code..."
codespell * || result=1
shopt -u dotglob

if [ -n "$CI_MERGE_REQUEST_DIFF_BASE_SHA" ]; then
  for COMMIT in $(git rev-list "^$CI_MERGE_REQUEST_DIFF_BASE_SHA" "$CI_COMMIT_SHA"); do
    echo "Running 'codespell' on commit message of $COMMIT..."
    git show --format=%B -s "$COMMIT" | codespell - || result=1
  done
fi

exit $result
