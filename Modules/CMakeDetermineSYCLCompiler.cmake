# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.


if(CMAKE_GENERATOR MATCHES "^Xcode")
  message(FATAL_ERROR
    "The SYCL language is not supported by the \"${CMAKE_GENERATOR}\" "
    "generator.")
endif()

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)

# Record the selection used to identify the host toolchain and runtime.
if(DEFINED CMAKE_SYCL_DEVICE_TARGETS)
  set(_CMAKE_SYCL_DEVICE_TARGETS "${CMAKE_SYCL_DEVICE_TARGETS}")
endif()

# Load system-specific compiler preferences for this language.
include(Platform/${CMAKE_SYSTEM_NAME}-Determine-SYCL OPTIONAL)
include(Platform/${CMAKE_SYSTEM_NAME}-SYCL OPTIONAL)
if(NOT CMAKE_SYCL_COMPILER_NAMES)
  set(CMAKE_SYCL_COMPILER_NAMES dpclang++ dpclang-cl icpx icx icx-cl dpcpp acpp syclcc clang++ clang-cl)
endif()

if(NOT "${CMAKE_GENERATOR}" MATCHES "Green Hills MULTI")
  if(NOT CMAKE_SYCL_COMPILER)
    set(CMAKE_SYCL_COMPILER_INIT NOTFOUND)

    if(NOT $ENV{SYCLCXX} STREQUAL "")
      get_filename_component(CMAKE_SYCL_COMPILER_INIT $ENV{SYCLCXX} PROGRAM PROGRAM_ARGS CMAKE_SYCL_FLAGS_ENV_INIT)
      if(CMAKE_SYCL_FLAGS_ENV_INIT)
        set(CMAKE_SYCL_COMPILER_ARG1 "${CMAKE_SYCL_FLAGS_ENV_INIT}" CACHE STRING "Arguments to SYCL compiler")
      endif()
      if(NOT EXISTS ${CMAKE_SYCL_COMPILER_INIT})
        message(FATAL_ERROR "Could not find the compiler specified in the environment variable SYCLCXX:\n$ENV{SYCLCXX}.\n${CMAKE_SYCL_COMPILER_INIT}")
      endif()
    endif()

    if(CMAKE_GENERATOR_SYCL AND NOT CMAKE_SYCL_COMPILER_INIT)
      set(CMAKE_SYCL_COMPILER_INIT ${CMAKE_GENERATOR_SYCL})
    endif()

    if(NOT CMAKE_SYCL_COMPILER_INIT)
      set(CMAKE_SYCL_COMPILER_LIST dpclang++ dpclang-cl icpx icx icx-cl dpcpp acpp syclcc clang++ clang-cl)
    endif()

    _cmake_find_compiler(SYCL)
  else()
    _cmake_find_compiler_path(SYCL)
  endif()
  mark_as_advanced(CMAKE_SYCL_COMPILER)

  get_filename_component(_CMAKE_SYCL_COMPILER_NAME
    "${CMAKE_SYCL_COMPILER}" NAME_WE)
  if(_CMAKE_SYCL_COMPILER_NAME MATCHES "^(acpp|syclcc|syclcc-clang)$")
    set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST "-x c++")
    set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS "-c -x c++" "-x c++")
  else()
    set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST "-fsycl -x c++")
    set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS
      "-c -fsycl -x c++"
      "-fsycl -x c++"
      "-c -x c++"
      "-x c++")
  endif()
  unset(_CMAKE_SYCL_COMPILER_NAME)
endif()

if(CMAKE_SYCL_COMPILER_TARGET)
  get_filename_component(_CMAKE_SYCL_COMPILER_NAME
    "${CMAKE_SYCL_COMPILER}" NAME_WE)
  if(_CMAKE_SYCL_COMPILER_NAME MATCHES "^(acpp|syclcc|syclcc-clang)$")
    set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST
      "-c --target=${CMAKE_SYCL_COMPILER_TARGET} -x c++")
  else()
    set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST
      "-c --target=${CMAKE_SYCL_COMPILER_TARGET} -fsycl -x c++")
  endif()
  unset(_CMAKE_SYCL_COMPILER_NAME)
endif()

get_filename_component(_CMAKE_SYCL_COMPILER_NAME
  "${CMAKE_SYCL_COMPILER}" NAME_WE)
if(_CMAKE_SYCL_COMPILER_NAME MATCHES "^(acpp|syclcc|syclcc-clang)$")
  if(NOT DEFINED _CMAKE_SYCL_DEVICE_TARGETS)
    if(DEFINED ENV{ACPP_TARGETS})
      set(_CMAKE_SYCL_DEVICE_TARGETS "$ENV{ACPP_TARGETS}")
    elseif(DEFINED ENV{HIPSYCL_TARGETS})
      set(_CMAKE_SYCL_DEVICE_TARGETS "$ENV{HIPSYCL_TARGETS}")
    else()
      execute_process(COMMAND "${CMAKE_SYCL_COMPILER}" --help
        OUTPUT_VARIABLE _CMAKE_SYCL_DRIVER_HELP
        ERROR_QUIET)
      if(_CMAKE_SYCL_DRIVER_HELP MATCHES
          "--acpp-targets=[^\n]*\n[^\n]*\n[^\n]*\n  \\[current value: ([^]\n]+)\\]")
        if(NOT CMAKE_MATCH_1 STREQUAL "NOT SET")
          set(_CMAKE_SYCL_DEVICE_TARGETS "${CMAKE_MATCH_1}")
        endif()
      endif()
      unset(_CMAKE_SYCL_DRIVER_HELP)
    endif()
  endif()
  # AdaptiveCpp supplies the backend's source language itself.  A forwarded
  # -x c++ would override its -x cuda/-x hip during multipass compilation.
  string(TOLOWER "${_CMAKE_SYCL_DEVICE_TARGETS}" _CMAKE_SYCL_TARGET_FLOWS)
  if(_CMAKE_SYCL_TARGET_FLOWS MATCHES
      "(^|;)[ \t]*(cuda|hip)(\\.(integrated|explicit)-multipass)?[ \t]*(:|;|$)" OR
      (NOT "${CMAKE_SYCL_DEVICE_TARGETS}" STREQUAL "" AND
       NOT CMAKE_SYCL_DEVICE_TARGETS))
    string(REPLACE "-x c++" "" CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST
      "${CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST}")
    string(REPLACE "-x c++" "" CMAKE_SYCL_COMPILER_ID_TEST_FLAGS
      "${CMAKE_SYCL_COMPILER_ID_TEST_FLAGS}")
  endif()
  unset(_CMAKE_SYCL_TARGET_FLOWS)
  set(_CMAKE_SYCL_DEVICE_TARGET_OPTION "--acpp-targets=")
  set(_CMAKE_SYCL_DEVICE_TARGET_VALUE "${_CMAKE_SYCL_DEVICE_TARGETS}")
elseif(_CMAKE_SYCL_COMPILER_NAME MATCHES "^(clang\\+\\+|clang|clang-cl)(-[0-9]+)?$")
  set(_CMAKE_SYCL_DEVICE_TARGET_OPTION "--offload-targets=")
  string(REPLACE ";" "," _CMAKE_SYCL_DEVICE_TARGET_VALUE
    "${_CMAKE_SYCL_DEVICE_TARGETS}")
else()
  set(_CMAKE_SYCL_DEVICE_TARGET_OPTION "-fsycl-targets=")
  string(REPLACE ";" "," _CMAKE_SYCL_DEVICE_TARGET_VALUE
    "${_CMAKE_SYCL_DEVICE_TARGETS}")
endif()
if(_CMAKE_SYCL_DEVICE_TARGETS)
  set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST
    "\"${_CMAKE_SYCL_DEVICE_TARGET_OPTION}${_CMAKE_SYCL_DEVICE_TARGET_VALUE}\" ${CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST}")
  string(REPLACE ";" "\\;" _CMAKE_SYCL_DEVICE_TARGET_VALUE
    "${_CMAKE_SYCL_DEVICE_TARGET_VALUE}")
  foreach(_flag IN LISTS CMAKE_SYCL_COMPILER_ID_TEST_FLAGS)
    # Subsequent attempts must not silently fall back to default targets.
    string(APPEND _CMAKE_SYCL_ID_TARGET_FLAGS
      "\"${_CMAKE_SYCL_DEVICE_TARGET_OPTION}${_CMAKE_SYCL_DEVICE_TARGET_VALUE}\" ${_flag};")
  endforeach()
  set(CMAKE_SYCL_COMPILER_ID_TEST_FLAGS "${_CMAKE_SYCL_ID_TARGET_FLAGS}")
endif()
unset(_CMAKE_SYCL_COMPILER_NAME)
unset(_CMAKE_SYCL_DEVICE_TARGET_OPTION)
unset(_CMAKE_SYCL_DEVICE_TARGET_VALUE)
unset(_CMAKE_SYCL_ID_TARGET_FLAGS)
unset(_flag)

if(NOT CMAKE_SYCL_COMPILER_ID_RUN)
  set(CMAKE_SYCL_COMPILER_ID_RUN 1)

  set(CMAKE_SYCL_COMPILER_ID)
  set(CMAKE_SYCL_PLATFORM_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_SYCL_COMPILER_ID_PLATFORM_CONTENT)

  list(APPEND CMAKE_SYCL_COMPILER_ID_VENDORS IAR)
  set(CMAKE_SYCL_COMPILER_ID_VENDOR_FLAGS_IAR )
  set(CMAKE_SYCL_COMPILER_ID_VENDOR_REGEX_IAR "IAR .+ Compiler")

  set(CMAKE_SYCL_COMPILER_ID_TOOL_MATCH_REGEX "\nLd[^\n]*(\n[ \t]+[^\n]*)*\n[ \t]+([^ \t\r\n]+)[^\r\n]*-o[^\r\n]*CompilerIdSYCL/(\\./)?(CompilerIdSYCL.(framework|xctest|build/[^ \t\r\n]+)/)?CompilerIdSYCL[ \t\n\\\"]")
  set(CMAKE_SYCL_COMPILER_ID_TOOL_MATCH_INDEX 2)

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  get_filename_component(_CMAKE_SYCL_COMPILER_NAME "${CMAKE_SYCL_COMPILER}" NAME_WE)
  if(_CMAKE_SYCL_COMPILER_NAME MATCHES "-cl$")
    string(APPEND CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST " -clang:-v")
  else()
    string(APPEND CMAKE_SYCL_COMPILER_ID_TEST_FLAGS_FIRST " -v")
  endif()
  unset(_CMAKE_SYCL_COMPILER_NAME)
  CMAKE_DETERMINE_COMPILER_ID(SYCL SYCLFLAGS CMakeSYCLCompilerId.sycl)

  include(CMakeParseImplicitLinkInfo)
  cmake_parse_implicit_link_info2("${CMAKE_SYCL_COMPILER_PRODUCED_OUTPUT}" _CMAKE_SYCL_LINK_LOG ""
    LANGUAGE SYCL
    COMPUTE_IMPLICIT_LIBS CMAKE_SYCL_HOST_IMPLICIT_LINK_LIBRARIES
    COMPUTE_IMPLICIT_DIRS CMAKE_SYCL_HOST_IMPLICIT_LINK_DIRECTORIES
    COMPUTE_IMPLICIT_FWKS CMAKE_SYCL_HOST_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES)
  message(CONFIGURE_LOG "Parsed SYCL compiler-id link information:\n${_CMAKE_SYCL_LINK_LOG}\n")
  set(CMAKE_SYCL_IMPLICIT_LINK_LIBRARIES "${CMAKE_SYCL_HOST_IMPLICIT_LINK_LIBRARIES}")
  set(CMAKE_SYCL_IMPLICIT_LINK_DIRECTORIES "${CMAKE_SYCL_HOST_IMPLICIT_LINK_DIRECTORIES}")
  set(CMAKE_SYCL_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES "${CMAKE_SYCL_HOST_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES}")
  include(Internal/CMakeSYCLFilterImplicitLibs)
  cmake_sycl_filter_implicit_libs(CMAKE_SYCL_IMPLICIT_LINK_LIBRARIES)

  _cmake_find_compiler_sysroot(SYCL)
else()
  if(NOT DEFINED CMAKE_SYCL_COMPILER_FRONTEND_VARIANT)
    if(CMAKE_SYCL_COMPILER_ID STREQUAL "Clang" OR
       "x${CMAKE_SYCL_COMPILER_ID}" STREQUAL "xIntelLLVM")
      if("x${CMAKE_SYCL_SIMULATE_ID}" STREQUAL "xMSVC")
        set(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT "MSVC")
      else()
        set(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT "GNU")
      endif()
    elseif(CMAKE_SYCL_COMPILER_ID STREQUAL "AdaptiveCpp")
      set(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT "GNU")
    else()
      set(CMAKE_SYCL_COMPILER_FRONTEND_VARIANT "")
    endif()
  endif()
endif()

if(NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_SYCL_COMPILER}" PATH)
endif()

if(NOT _CMAKE_TOOLCHAIN_PREFIX)
  if(CMAKE_SYCL_COMPILER_ID MATCHES "GNU|Clang|IntelLLVM|QCC|LCC")
    get_filename_component(COMPILER_BASENAME "${CMAKE_SYCL_COMPILER}" NAME)
    if(COMPILER_BASENAME MATCHES "^(.+-)?(dpclang\\+\\+|dpclang-cl|dpclang|clang\\+\\+|dpcpp|icpx|[gc]\\+\\+|clang-cl)(-?[0-9]+(\\.[0-9]+)*)?(-[^.]+)?(\\.exe)?$")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
      set(_CMAKE_TOOLCHAIN_SUFFIX ${CMAKE_MATCH_3})
      set(_CMAKE_COMPILER_SUFFIX ${CMAKE_MATCH_5})
    elseif(CMAKE_SYCL_COMPILER_TARGET AND CMAKE_SYCL_COMPILER_ID MATCHES "Clang|IntelLLVM")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_SYCL_COMPILER_TARGET}-)
    endif()

    if("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
    endif()
  endif()
endif()

set(_CMAKE_PROCESSING_LANGUAGE "SYCL")
include(CMakeFindBinUtils)
include(Compiler/${CMAKE_SYCL_COMPILER_ID}-FindBinUtils OPTIONAL)
unset(_CMAKE_PROCESSING_LANGUAGE)

if(CMAKE_SYCL_COMPILER_SYSROOT)
  string(CONCAT _SET_CMAKE_SYCL_COMPILER_SYSROOT
    "set(CMAKE_SYCL_COMPILER_SYSROOT \"${CMAKE_SYCL_COMPILER_SYSROOT}\")\n"
    "set(CMAKE_COMPILER_SYSROOT \"${CMAKE_SYCL_COMPILER_SYSROOT}\")")
else()
  set(_SET_CMAKE_SYCL_COMPILER_SYSROOT "")
endif()

if(MSVC_SYCL_ARCHITECTURE_ID)
  set(SET_MSVC_SYCL_ARCHITECTURE_ID
    "set(MSVC_SYCL_ARCHITECTURE_ID ${MSVC_SYCL_ARCHITECTURE_ID})")
endif()

# configure all variables set in this file
configure_file(${CMAKE_ROOT}/Modules/CMakeSYCLCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeSYCLCompiler.cmake
  @ONLY
  )

set(CMAKE_SYCL_COMPILER_ENV_VAR "SYCLCXX")
