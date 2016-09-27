#!/usr/bin/env bash

# Filter scripts.

regex='#=====================*
(# CMake - Cross Platform Makefile Generator
)?(# Copyright.*
)*#
# Distributed under the OSI-approved BSD License \(the "License"\);
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=====================*(
# \(To distribute this file outside of CMake, substitute the full
#  License text for the above reference.\))?
+'
files="$(git ls-files -- | egrep -v ' ' | xargs pcregrep -M -l "$regex")"


if test "x$files" != "x"; then
  sed -i '1 i# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying\
# file Copyright.txt or https://cmake.org/licensing for details.\

' $files

  perl -i -0pe "s/$regex//" -- $files
fi

# Filter C and C++ sources.

regex='\/\*=======================*(
  .*)+

  Distributed under the OSI-approved BSD License \(the "License"\);
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
=======================*\*\/
+'

notice='\/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https:\/\/cmake.org\/licensing for details.  *\/
'

git ls-files -z -- | grep -z -v 'Source/kwsys' | xargs -0 perl -i -0pe "s/$regex/$notice/g"
