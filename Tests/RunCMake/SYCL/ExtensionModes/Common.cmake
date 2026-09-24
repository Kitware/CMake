set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(SYCL)

string(APPEND CMAKE_CXX_FLAGS " -DCXX_LANGUAGE_FLAG")
string(APPEND CMAKE_SYCL_FLAGS " -DSYCL_LANGUAGE_FLAG")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(add_extension_mode_target name)
  add_executable(${name} main.sycl feature.cxx)
  set_target_properties(${name} PROPERTIES
    SYCL_CXX_STANDARD 17 SYCL_CXX_STANDARD_REQUIRED ON)
  if(ARGC GREATER 1)
    set_target_properties(${name} PROPERTIES SYCL_EXTENSION_MODE "${ARGV1}")
  endif()
  if(ARGV1 STREQUAL "REPLACE")
    target_compile_definitions(${name} PRIVATE EXPECT_REPLACE)
  else()
    target_compile_definitions(${name} PRIVATE EXPECT_APPEND)
  endif()
  add_test(NAME ${name} COMMAND ${name})
endfunction()

enable_testing()
