# Disable warnings to avoid changing 3rd party code.
IF(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|Clang|AppleClang|XL|VisualAge|SunPro|MIPSpro|HP|Intel)$")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
ENDIF()

include(ConfigureChecks.cmake)
if(WIN32)
  add_definitions(-DCOMPILING_FOR_WINDOWS)
endif(WIN32)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  )

add_library(cmexpat STATIC
  lib/xmlparse.c
  lib/xmlrole.c
  lib/xmltok.c
  lib/xmltok_impl.c
  lib/xmltok_ns.c
  )
