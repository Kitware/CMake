#!/bin/sh

set -e

result=0

echo "Running 'typos' on source code..."
typos || result=1

cfg='.typos.toml'
tmp_cfg="${TEMP:-/tmp}/$cfg"
# Uncomment `extend-ignore-identifiers-re` in the top-level config file
# to make Git hashes (possibly used in commit messages) valid "identifiers".
sed 's/^#\s*\(extend-ignore-identifiers-re\)/\1/' "$cfg" >"$tmp_cfg"

if [ -n "$CI_MERGE_REQUEST_DIFF_BASE_SHA" ]; then
  for COMMIT in $(git rev-list "^$CI_MERGE_REQUEST_DIFF_BASE_SHA" "$CI_COMMIT_SHA"); do
    echo "Running 'typos' on commit message of $COMMIT..."
    git show --format=%B -s "$COMMIT" | typos -c "$tmp_cfg" - || result=1
  done
fi

exit $result
