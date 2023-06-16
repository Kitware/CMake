project(bzip2)

# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|LCC|Clang|AppleClang|IBMClang|XLClang|XL|VisualAge|SunPro|HP|Intel|IntelLLVM|NVHPC)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

# Activate POSIX APIs.
if(CMAKE_SYSTEM_NAME MATCHES "^(AIX|OS400)$")
  add_definitions(-D_ALL_SOURCE)
endif()
if(NOT CMAKE_SYSTEM_NAME MATCHES "BSD|Darwin|Windows")
  add_definitions(-D_XOPEN_SOURCE=600)
endif()

add_definitions(-D_FILE_OFFSET_BITS=64)
add_library(cmbzip2
  blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c)

if(WIN32 AND CMake_BUILD_PCH)
  target_precompile_headers(cmbzip2 PRIVATE "bzlib.h")
endif()
