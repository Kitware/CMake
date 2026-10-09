# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


if(CMAKE_SYCL_COMPILER_FORCED)
  set(CMAKE_SYCL_COMPILER_WORKS TRUE)
  return()
endif()

include(CMakeTestCompilerCommon)

set(__CMAKE_SAVED_TRY_COMPILE_TARGET_TYPE ${CMAKE_TRY_COMPILE_TARGET_TYPE})
if(_CMAKE_FEATURE_DETECTION_TARGET_TYPE)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE ${_CMAKE_FEATURE_DETECTION_TARGET_TYPE})
endif()

unset(CMAKE_SYCL_COMPILER_WORKS CACHE)

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerABI.cmake)
CMAKE_DETERMINE_COMPILER_ABI(SYCL ${CMAKE_ROOT}/Modules/CMakeSYCLCompilerABI.sycl)

if(CMAKE_SYCL_ABI_COMPILED)
  set(CMAKE_SYCL_COMPILER_WORKS TRUE)
  message(STATUS "Check for working SYCL compiler: ${CMAKE_SYCL_COMPILER} - skipped")
endif()

if(NOT CMAKE_SYCL_COMPILER_WORKS)
  PrintTestCompilerStatus("SYCL")
  __TestCompiler_setTryCompileTargetType()
  string(CONCAT __TestCompiler_testSYCLCompilerSource
    "#ifndef __cplusplus\n"
    "# error \"The CMAKE_SYCL_COMPILER is not compiling as C++\"\n"
    "#endif\n"
    "#include <sycl/sycl.hpp>\n"
    "class CMakeSYCLCompilerTest;\n"
    "int main() {\n"
    "  sycl::queue queue;\n"
    "  queue.single_task<CMakeSYCLCompilerTest>([] {}).wait();\n"
    "  return 0;\n"
    "}\n")
  unset(CMAKE_SYCL_COMPILER_WORKS)
  try_compile(CMAKE_SYCL_COMPILER_WORKS
    SOURCE_FROM_VAR testSYCLCompiler.sycl __TestCompiler_testSYCLCompilerSource
    NO_CACHE
    OUTPUT_VARIABLE __CMAKE_SYCL_COMPILER_OUTPUT)
  __TestCompiler_restoreTryCompileTargetType()
  unset(__TestCompiler_testSYCLCompilerSource)
  if(NOT CMAKE_SYCL_COMPILER_WORKS)
    PrintTestCompilerResult(CHECK_FAIL "broken")
    string(REPLACE "\n" "\n  " _output "${__CMAKE_SYCL_COMPILER_OUTPUT}")
    message(FATAL_ERROR "The SYCL compiler\n  \"${CMAKE_SYCL_COMPILER}\"\n"
      "is not able to compile a SYCL test program.\nIt fails "
      "with the following output:\n  ${_output}\n\n"
      "CMake will not be able to correctly generate this project.")
  endif()
  PrintTestCompilerResult(CHECK_PASS "works")
endif()

if(NOT CMAKE_SYCL_IMPLICIT_LINK_LIBRARIES)
  set(CMAKE_SYCL_IMPLICIT_LINK_LIBRARIES "${CMAKE_SYCL_HOST_IMPLICIT_LINK_LIBRARIES}")
  set(CMAKE_SYCL_IMPLICIT_LINK_DIRECTORIES "${CMAKE_SYCL_HOST_IMPLICIT_LINK_DIRECTORIES}")
  set(CMAKE_SYCL_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES "${CMAKE_SYCL_HOST_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES}")
endif()
include(Internal/CMakeSYCLFilterImplicitLibs)
cmake_sycl_filter_implicit_libs(CMAKE_SYCL_IMPLICIT_LINK_LIBRARIES)

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerSupport.cmake)
CMAKE_DETERMINE_COMPILER_SUPPORT(SYCL)

configure_file(
  ${CMAKE_ROOT}/Modules/CMakeSYCLCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeSYCLCompiler.cmake
  @ONLY
  )
include(${CMAKE_PLATFORM_INFO_DIR}/CMakeSYCLCompiler.cmake)

if(CMAKE_SYCL_SIZEOF_DATA_PTR)
  foreach(f ${CMAKE_SYCL_ABI_FILES})
    include(${f})
  endforeach()
  unset(CMAKE_SYCL_ABI_FILES)
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE ${__CMAKE_SAVED_TRY_COMPILE_TARGET_TYPE})
unset(__CMAKE_SAVED_TRY_COMPILE_TARGET_TYPE)
unset(__CMAKE_SYCL_COMPILER_OUTPUT)
