cmake_minimum_required(VERSION 2.8)

project(CompileOptions)

add_library(testlib other.cpp)

if(TEST_FORTRAN)
  enable_language(Fortran)
endif()

add_executable(CompileOptions main.cpp)

macro(get_compiler_test_genex lst lang)
  list(APPEND ${lst} -DTEST_${lang}_COMPILER_VERSION="$<${lang}_COMPILER_VERSION>")
  list(APPEND ${lst} -DTEST_${lang}_COMPILER_VERSION_EQUALITY=$<${lang}_COMPILER_VERSION:${CMAKE_${lang}_COMPILER_VERSION}>)
endmacro()

get_compiler_test_genex(c_tests C)
get_compiler_test_genex(cxx_tests CXX)
if(TEST_FORTRAN)
  get_compiler_test_genex(fortran_tests Fortran)
endif()

set_property(TARGET CompileOptions PROPERTY COMPILE_OPTIONS
  "-DTEST_DEFINE"
  "-DNEEDS_ESCAPE=\"E$CAPE\""
  "$<$<CXX_COMPILER_ID:GNU>:-DTEST_DEFINE_GNU>"
  "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-DTEST_DEFINE_CXX_AND_GNU>"
  "SHELL:" # produces no options
  ${c_tests}
  ${cxx_tests}
  ${fortran_tests}
  )
if(BORLAND OR WATCOM)
  # these compilers do not support separate -D flags
  target_compile_definitions(CompileOptions PRIVATE NO_DEF_TESTS)
else()
  set_property(TARGET CompileOptions APPEND PROPERTY COMPILE_OPTIONS
    "SHELL:-D DEF_A"
    "$<1:SHELL:-D DEF_B>"
    "SHELL:-D 'DEF_C' -D \"DEF_D\""
    [[SHELL:-D "DEF_STR=\"string with spaces\""]]
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|Borland|Embarcadero" AND NOT "${CMAKE_GENERATOR}" MATCHES "NMake Makefiles")
  set_property(TARGET CompileOptions APPEND PROPERTY COMPILE_OPTIONS
    "-DTEST_OCTOTHORPE=\"#\""
    )
endif()

target_link_libraries(CompileOptions testlib)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_definitions(CompileOptions
    PRIVATE
      "DO_GNU_TESTS"
  )
endif()

target_compile_definitions(CompileOptions
  PRIVATE
    "EXPECTED_C_COMPILER_VERSION=\"${CMAKE_C_COMPILER_VERSION}\""
    "EXPECTED_CXX_COMPILER_VERSION=\"${CMAKE_CXX_COMPILER_VERSION}\""
)

if(TEST_FORTRAN)
  # Definitions for the C++ code to test the values
  target_compile_definitions(CompileOptions
    PRIVATE
      "TEST_FORTRAN"
      "EXPECTED_Fortran_COMPILER_VERSION=\"${CMAKE_Fortran_COMPILER_VERSION}\""
  )
endif()
