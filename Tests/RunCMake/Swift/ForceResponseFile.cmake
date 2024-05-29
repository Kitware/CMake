if(POLICY CMP0157)
  cmake_policy(SET CMP0157 NEW)
endif()

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(SEND_ERROR "this test must use a Ninja generator, found ${CMAKE_GENERATOR} ")
endif()

set(CMAKE_NINJA_FORCE_RESPONSE_FILE TRUE)

enable_language(Swift)

add_library(L STATIC L.swift)
