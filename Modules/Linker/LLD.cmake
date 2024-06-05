# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include_guard()

include(Linker/GNU)

macro(__linker_lld lang)
  __linker_gnu(${lang})
endmacro()
