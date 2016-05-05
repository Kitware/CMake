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
