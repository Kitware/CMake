cmake_policy(SET CMP0157 NEW)
cmake_policy(SET CMP0195 NEW)
cmake_policy(SET CMP0215 NEW)

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(SEND_ERROR "this test must use a Ninja generator, found ${CMAKE_GENERATOR}")
endif()

enable_language(Swift)

# A standalone library with a custom module output path (like swiftCore
# in the Swift stdlib).  The link edge must depend on the actual
# emit-module output, not GetSwiftModulePath().
add_library(L STATIC L.swift)
set_target_properties(L PROPERTIES
  Swift_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/custom)
