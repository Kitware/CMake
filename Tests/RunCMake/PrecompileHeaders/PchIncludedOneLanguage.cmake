enable_language(C)
enable_language(CXX)

if(CMAKE_CXX_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

add_executable(main
  main.cpp
  empty.c
  pch-included.cpp
)

target_precompile_headers(main PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>)

enable_testing()
add_test(NAME main COMMAND main)
