#!/usr/bin/env bash
# Update the version component if it looks like a date or -f is given.
version_file="${BASH_SOURCE%/*}/CMakeVersion.cmake"
if test "x$1" = "x-f"; then shift ; n='*' ; else n='\{8\}' ; fi
if test "$#" -gt 0; then echo 1>&2 "usage: CMakeVersion.bash [-f]"; exit 1; fi
sed -i -e '
s/\(^set(CMake_VERSION_PATCH\) [0-9]'"$n"'\(.*\)/\1 '"$(date +%Y%m%d)"'\2/
' "$version_file"
# Update the copyright notice to match the version date's year.
if version_patch_line=$(grep -E '^set\(CMake_VERSION_PATCH [0-9]{8}\)' "$version_file"); then
  version_patch_year="${version_patch_line:24:4}"
  if [[ "$version_patch_year" =~ ^[0-9][0-9][0-9][0-9]$ ]] ; then
    sed -i -e '
      s/\(^Copyright 2000-\)[0-9][0-9][0-9][0-9]\( .*\)/\1'"$version_patch_year"'\2/
    ' "${BASH_SOURCE%/*}/../LICENSE.rst"
  fi
fi
