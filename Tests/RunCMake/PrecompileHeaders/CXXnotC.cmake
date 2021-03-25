enable_language(C)
enable_language(CXX)

add_executable(main
  no_pch.c
  use_pch.cxx
)

target_include_directories(main PUBLIC include)
target_precompile_headers(main PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/cxx_pch.h>"
  )

enable_testing()
add_test(NAME main COMMAND main)
