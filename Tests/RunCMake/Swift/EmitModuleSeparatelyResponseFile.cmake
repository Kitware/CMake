cmake_policy(SET CMP0157 NEW)
cmake_policy(SET CMP0195 NEW)
cmake_policy(SET CMP0215 NEW)

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(SEND_ERROR "this test must use a Ninja generator, found ${CMAKE_GENERATOR} ")
endif()

set(CMAKE_NINJA_FORCE_RESPONSE_FILE TRUE)

enable_language(Swift)

# Older Swift compilers may not set CMAKE_Swift_MODULE_TRIPLE
# Don't build anything if we can't do the nested module structure
if(NOT CMAKE_Swift_MODULE_TRIPLE)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/no-swift-module-triple" "")
  return()
endif()

add_library(L STATIC L.swift)
add_library(LClient STATIC LClient.swift)
target_link_libraries(LClient PRIVATE L)
