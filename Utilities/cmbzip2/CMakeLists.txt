project(bzip2)

# Disable warnings to avoid changing 3rd party code.
if(CMAKE_C_COMPILER_ID MATCHES
    "^(GNU|Clang|AppleClang|XLClang|XL|VisualAge|SunPro|HP|Intel)$")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
elseif(CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -woffall")
endif()

add_definitions(-D_FILE_OFFSET_BITS=64)
add_library(cmbzip2
  blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c)
