cmake_minimum_required(VERSION 3.0)
if(POLICY CMP0092)
  cmake_policy(SET CMP0092 NEW)
endif()
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _isMultiConfig AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()
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
  "$<$<CXX_COMPILER_ID:GNU,LCC>:-DTEST_DEFINE_GNU>"
  "$<$<COMPILE_LANG_AND_ID:CXX,GNU,LCC>:-DTEST_DEFINE_CXX_AND_GNU>"
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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC|Clang|Borland|Embarcadero" AND NOT "${CMAKE_GENERATOR}" MATCHES "NMake Makefiles")
  set_property(TARGET CompileOptions APPEND PROPERTY COMPILE_OPTIONS
    "-DTEST_OCTOTHORPE=\"#\""
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|LCC|AppleClang|MSVC)$")
  target_compile_definitions(CompileOptions PRIVATE "DO_FLAG_TESTS")
  if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|LCC|AppleClang)$")
    string(APPEND CMAKE_CXX_FLAGS " -w")
  endif()
  string(APPEND CMAKE_CXX_FLAGS                " -DFLAG_A=1 -DFLAG_B=1")
  string(APPEND CMAKE_CXX_FLAGS_DEBUG          " -DFLAG_A=2 -DFLAG_C=1")
  string(APPEND CMAKE_CXX_FLAGS_RELEASE        " -DFLAG_A=2 -DFLAG_C=1")
  string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -DFLAG_A=2 -DFLAG_C=1")
  string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL     " -DFLAG_A=2 -DFLAG_C=1")
  string(TOUPPER "${CMAKE_BUILD_TYPE}" _xbuild_type)
  if(NOT _xbuild_type MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    string(APPEND CMAKE_CXX_FLAGS_${_xbuild_type} " -DFLAG_A=2 -DFLAG_C=1")
  endif()
  set_property(TARGET CompileOptions APPEND PROPERTY COMPILE_OPTIONS "-DFLAG_B=2" "-DFLAG_C=2" "-DFLAG_D=1")
  set_property(TARGET testlib APPEND PROPERTY INTERFACE_COMPILE_OPTIONS "-DFLAG_D=2")
  if(NOT CMAKE_GENERATOR MATCHES "^Visual Studio 9")
    set_property(TARGET testlib APPEND PROPERTY INTERFACE_COMPILE_OPTIONS "-DFLAG_E=1")
    set_property(SOURCE main.cpp PROPERTY COMPILE_OPTIONS "-DFLAG_E=2")
  endif()
endif()

target_link_libraries(CompileOptions testlib)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC")
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
