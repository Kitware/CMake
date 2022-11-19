enable_language(C)
enable_language(CXX)

add_executable(foobar
  foo.c
  main.cpp
)
target_include_directories(foobar PUBLIC include)
target_precompile_headers(foobar PRIVATE
  "$<$<COMPILE_LANGUAGE:C>:${CMAKE_CURRENT_SOURCE_DIR}/include/foo_C.h>"
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/foo_CXX.h>"
  "$<$<COMPILE_LANGUAGE:C>:<stddef.h$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<cstddef$<ANGLE-R>>"
  )
