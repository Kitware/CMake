enable_language(CXX)

# Write value of `SET_CHARSET` for comparison later.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/set_charset.txt" "${SET_CHARSET}")

# Set macro which determines the character-set.
if("${SET_CHARSET}" STREQUAL "MultiByte")
  add_compile_definitions(_MBCS=1)
endif()
if("${SET_CHARSET}" STREQUAL "NotSet")
  add_compile_definitions(_SBCS=1)
endif()
if("${SET_CHARSET}" STREQUAL "Unicode")
  add_compile_definitions(_UNICODE=1)
endif()

add_library(foo foo.cpp)
