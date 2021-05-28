
cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)

set(SWIG_EXECUTABLE "swig")
set(SWIG_DIR "/swig")
include(UseSWIG)

swig_add_library(example LANGUAGE csharp TYPE SHARED SOURCES example.i)

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/CMP0122-library-name.txt"
     CONTENT "${CMAKE_SHARED_LIBRARY_PREFIX}\n$<TARGET_FILE_PREFIX:example>\n")
