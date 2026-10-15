set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)
enable_language(CXX)

if(CMAKE_CXX_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

# A force include added here lands in the directory define flags, which are
# a source of compile flags separate from the target compile options below.
add_definitions(-include forced.h)

# A force include whose path needs quoting, to cover the spelling in which
# the whole argument rather than just the file name is quoted.
set(forced4_dir "${CMAKE_CURRENT_BINARY_DIR}/forced dir")
file(WRITE "${forced4_dir}/forced4.h" "#define FORCED4_INCLUDED 1\n")

add_executable(main pch-forced.cpp pch-forced.c)
target_include_directories(main PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )

# The precompiled header is requested for CXX only, but the Xcode generator
# expresses it as the target-wide 'GCC_PREFIX_HEADER' build setting, so the C
# source is affected too.
target_precompile_headers(main PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch-forced.h>"
  )

# The precompiled header must be used even though other headers are
# force-included before it, in any of the spellings Clang accepts.
target_compile_options(main PRIVATE
  -include forced2.h
  -includeforced3.h
  "-include${forced4_dir}/forced4.h"
  --include=forced5.h
  )

enable_testing()
add_test(NAME main COMMAND main)
