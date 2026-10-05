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
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  add_definitions(-D_XOPEN_SOURCE=600)
endif()

include(ConfigureChecks.cmake)
configure_file(expat_config.h.cmake expat_config.h @ONLY)

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
if(HAVE_ARC4RANDOM)
  target_sources(cmexpat PRIVATE
    lib/random_arc4random.c
    lib/random_arc4random.h
  )
endif()

if(HAVE_ARC4RANDOM_BUF)
  target_sources(cmexpat PRIVATE
    lib/random_arc4random_buf.c
    lib/random_arc4random_buf.h
  )
endif()

if(HAVE_GETENTROPY)
  target_sources(cmexpat PRIVATE
    lib/random_getentropy.c
    lib/random_getentropy.h
  )
endif()

if(HAVE_GETRANDOM OR HAVE_SYSCALL_GETRANDOM)
  target_sources(cmexpat PRIVATE
    lib/random_getrandom.c
    lib/random_getrandom.h
  )
endif()

if(WIN32)
  target_sources(cmexpat PRIVATE
    lib/random_rand_s.c
    lib/random_rand_s.h
  )
else()
  target_sources(cmexpat PRIVATE
    lib/random_dev_urandom.c
    lib/random_dev_urandom.h
  )
  target_compile_definitions(cmexpat PRIVATE XML_DEV_URANDOM)
endif()
