# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.
include_guard()

macro(__compiler_pellesc lang)
  set(CMAKE_${lang}_LINK_MODE LINKER)
endmacro ()
