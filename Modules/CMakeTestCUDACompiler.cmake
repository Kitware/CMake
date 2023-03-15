# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

if(CMAKE_CUDA_COMPILER_FORCED)
  # The compiler configuration was forced by the user.
  # Assume the user has configured all compiler information.
  set(CMAKE_CUDA_COMPILER_WORKS TRUE)
  return()
endif()

include(CMakeTestCompilerCommon)

# Remove any cached result from an older CMake version.
# We now store this in CMakeCUDACompiler.cmake.
unset(CMAKE_CUDA_COMPILER_WORKS CACHE)

# Try to identify the ABI and configure it into CMakeCUDACompiler.cmake
include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerABI.cmake)
CMAKE_DETERMINE_COMPILER_ABI(CUDA ${CMAKE_ROOT}/Modules/CMakeCUDACompilerABI.cu)
if(CMAKE_CUDA_ABI_COMPILED)
  # The compiler worked so skip dedicated test below.
  set(CMAKE_CUDA_COMPILER_WORKS TRUE)
  message(STATUS "Check for working CUDA compiler: ${CMAKE_CUDA_COMPILER} - skipped")

  # Run the test binary to detect the native architectures.
  execute_process(COMMAND "${CMAKE_PLATFORM_INFO_DIR}/CMakeDetermineCompilerABI_CUDA.bin"
    RESULT_VARIABLE _CUDA_ARCHS_RESULT
    OUTPUT_VARIABLE _CUDA_ARCHS_OUTPUT
    ERROR_VARIABLE  _CUDA_ARCHS_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(_CUDA_ARCHS_RESULT EQUAL 0)
    if("$ENV{CMAKE_CUDA_ARCHITECTURES_NATIVE_CLAMP}")
      # Undocumented hook used by CMake's CI.
      # Clamp native architecture to version range supported by this CUDA.
      list(GET CMAKE_CUDA_ARCHITECTURES_ALL 0  _CUDA_ARCH_MIN)
      list(GET CMAKE_CUDA_ARCHITECTURES_ALL -1 _CUDA_ARCH_MAX)
      set(CMAKE_CUDA_ARCHITECTURES_NATIVE "")
      foreach(_CUDA_ARCH IN LISTS _CUDA_ARCHS_OUTPUT)
        if(_CUDA_ARCH LESS _CUDA_ARCH_MIN)
          set(_CUDA_ARCH "${_CUDA_ARCH_MIN}")
        endif()
        if(_CUDA_ARCH GREATER _CUDA_ARCH_MAX)
          set(_CUDA_ARCH "${_CUDA_ARCH_MAX}")
        endif()
        list(APPEND CMAKE_CUDA_ARCHITECTURES_NATIVE ${_CUDA_ARCH})
      endforeach()
      unset(_CUDA_ARCH)
      unset(_CUDA_ARCH_MIN)
      unset(_CUDA_ARCH_MAX)
    else()
      set(CMAKE_CUDA_ARCHITECTURES_NATIVE "${_CUDA_ARCHS_OUTPUT}")
    endif()
    list(REMOVE_DUPLICATES CMAKE_CUDA_ARCHITECTURES_NATIVE)
    list(TRANSFORM CMAKE_CUDA_ARCHITECTURES_NATIVE APPEND "-real")
  else()
    if(NOT _CUDA_ARCHS_RESULT MATCHES "[0-9]+")
      set(_CUDA_ARCHS_STATUS " (${_CUDA_ARCHS_RESULT})")
    else()
      set(_CUDA_ARCHS_STATUS "")
    endif()
    string(REPLACE "\n" "\n  " _CUDA_ARCHS_OUTPUT "  ${_CUDA_ARCHS_OUTPUT}")
    message(CONFIGURE_LOG
      "Detecting the CUDA native architecture(s) failed with "
      "the following output:\n${_CUDA_ARCHS_OUTPUT}\n\n")
  endif()
  unset(_CUDA_ARCHS_EXE)
  unset(_CUDA_ARCHS_RESULT)
  unset(_CUDA_ARCHS_OUTPUT)
endif()

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that the selected cuda compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
if(NOT CMAKE_CUDA_COMPILER_WORKS)
  PrintTestCompilerStatus("CUDA")
  string(CONCAT __TestCompiler_testCudaCompilerSource
    "#ifndef __CUDACC__\n"
    "# error \"The CMAKE_CUDA_COMPILER is set to an invalid CUDA compiler\"\n"
    "#endif\n"
    "int main(){return 0;}\n")

  # Clear result from normal variable.
  unset(CMAKE_CUDA_COMPILER_WORKS)

  # Puts test result in cache variable.
  try_compile(CMAKE_CUDA_COMPILER_WORKS
    SOURCE_FROM_VAR main.cu __TestCompiler_testCudaCompilerSource
    OUTPUT_VARIABLE __CMAKE_CUDA_COMPILER_OUTPUT)
  unset(__TestCompiler_testCudaCompilerSource)

  # Move result from cache to normal variable.
  set(CMAKE_CUDA_COMPILER_WORKS ${CMAKE_CUDA_COMPILER_WORKS})
  unset(CMAKE_CUDA_COMPILER_WORKS CACHE)
  if(NOT CMAKE_CUDA_COMPILER_WORKS)
    PrintTestCompilerResult(CHECK_FAIL "broken")
    string(REPLACE "\n" "\n  " _output "${__CMAKE_CUDA_COMPILER_OUTPUT}")
    message(FATAL_ERROR "The CUDA compiler\n  \"${CMAKE_CUDA_COMPILER}\"\n"
      "is not able to compile a simple test program.\nIt fails "
      "with the following output:\n  ${_output}\n\n"
      "CMake will not be able to correctly generate this project.")
  endif()
  PrintTestCompilerResult(CHECK_PASS "works")
endif()

# Try to identify the compiler features
include(${CMAKE_ROOT}/Modules/CMakeDetermineCompileFeatures.cmake)
CMAKE_DETERMINE_COMPILE_FEATURES(CUDA)

if("x${CMAKE_CUDA_SIMULATE_ID}" STREQUAL "xMSVC")
  set(CMAKE_CUDA_IMPLICIT_LINK_LIBRARIES "${CMAKE_CUDA_HOST_IMPLICIT_LINK_LIBRARIES}")
  set(CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES "${CMAKE_CUDA_HOST_IMPLICIT_LINK_DIRECTORIES}")
endif()

# Filter out implicit link libraries that should not be passed unconditionally.
# See CMAKE_CUDA_IMPLICIT_LINK_LIBRARIES_EXCLUDE in CMakeDetermineCUDACompiler.
list(REMOVE_ITEM CMAKE_CUDA_IMPLICIT_LINK_LIBRARIES ${CMAKE_CUDA_IMPLICIT_LINK_LIBRARIES_EXCLUDE})

if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  # Remove the CUDA Toolkit include directories from the set of
  # implicit system include directories.
  # This resolves the issue that NVCC doesn't specify these
  # includes as SYSTEM includes when compiling device code, and sometimes
  # they contain headers that generate warnings, so let users mark them
  # as SYSTEM explicitly
  if(CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES)
    list(REMOVE_ITEM CMAKE_CUDA_IMPLICIT_INCLUDE_DIRECTORIES
      ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
      )
  endif()
endif()

# Re-configure to save learned information.
configure_file(
  ${CMAKE_ROOT}/Modules/CMakeCUDACompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeCUDACompiler.cmake
  @ONLY
  )
include(${CMAKE_PLATFORM_INFO_DIR}/CMakeCUDACompiler.cmake)

unset(__CMAKE_CUDA_COMPILER_OUTPUT)
