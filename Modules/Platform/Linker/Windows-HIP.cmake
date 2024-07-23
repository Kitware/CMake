# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0054 NEW)

if(CMAKE_HIP_SIMULATE_ID STREQUAL "MSVC")
  # MSVC is the default linker
  include(Platform/Linker/Windows-MSVC-HIP)
else()
    # GNU is the default linker
  include(Platform/Linker/Windows-GNU-HIP)
endif()

endblock()
