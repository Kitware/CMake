# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


# This module is shared by multiple linkers; use include blocker.
include_guard()

macro(__linker_wild lang)
  include(Linker/GNU)

  __linker_gnu(${lang})
endmacro()
