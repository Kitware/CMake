# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
include_guard()


macro(__windows_linker_msvc lang)
  set(CMAKE_${lang}_LINK_LIBRARIES_PROCESSING ORDER=FORWARD DEDUPLICATION=ALL)
endmacro()
