git config user.name "Kitware Robot"
git config user.email "kwrobot@kitware.com"

clang-apply-replacements --style=file .gitlab/clang-tidy-fixes
git add .

if [ -n "$(git status --porcelain)" ]; then
  quietly git commit --file=- <<EOF
WIP: clang-tidy: <SHORT DESCRIPTION OF CHANGE HERE>

<LONGER DESCRIPTION OF CHANGE HERE.>
EOF
  git format-patch --output=clang-tidy-fixes.patch -1 -N
  echo "Patch from clang-tidy available, check artifacts of this CI job." >&2
fi

readonly num_warnings="$(cat .gitlab/num_warnings.txt)"
if [ "$num_warnings" -ne 0 ]; then
  echo "Found $num_warnings warnings (treating as fatal)." >&2
  exit 1
fi
