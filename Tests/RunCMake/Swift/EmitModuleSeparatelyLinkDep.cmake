cmake_policy(SET CMP0157 NEW)
cmake_policy(SET CMP0195 NEW)
cmake_policy(SET CMP0215 NEW)

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(SEND_ERROR "this test must use a Ninja generator, found ${CMAKE_GENERATOR}")
endif()

enable_language(Swift)

# A standalone library with no dependent Swift targets.  The link edge
# must still depend on the .swiftmodule so the emit-module edge runs.
add_library(L STATIC L.swift)
