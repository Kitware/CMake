# Disable warnings to avoid changing 3rd party code.
IF(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
ENDIF()

# Activate POSIX APIs.
if(CMAKE_SYSTEM_NAME MATCHES "^(Linux)$")
  add_definitions(-D_DEFAULT_SOURCE -D_BSD_SOURCE)
  string(APPEND CMAKE_REQUIRED_DEFINITIONS " -D_DEFAULT_SOURCE -D_BSD_SOURCE")
endif()

include(ConfigureChecks.cmake)
configure_file(expat_config.h.cmake expat_config.h @ONLY)

if(NOT WIN32)
  add_definitions(-DXML_DEV_URANDOM)
endif()

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
