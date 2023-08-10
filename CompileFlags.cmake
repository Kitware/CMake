# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#-----------------------------------------------------------------------------
# set some special flags for different compilers
#
if(WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "Intel")
  set(_INTEL_WINDOWS 1)
endif()

if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "^(Clang|IntelLLVM)$"
   AND "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
  set(_CLANG_MSVC_WINDOWS 1)
endif()

# Disable deprecation warnings for standard C functions.
# really only needed for newer versions of VS, but should
# not hurt other versions, and this will work into the
# future
if(MSVC OR _INTEL_WINDOWS OR _CLANG_MSVC_WINDOWS)
  add_compile_definitions(_CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
else()
endif()

# MSVC 14.28 enables C5105, but the Windows SDK 10.0.18362.0 triggers it.
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 19.28)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -wd5105")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd5105")
endif()

# Use a stack size large enough for CMake_DEFAULT_RECURSION_LIMIT.
if(MSVC)
  string(APPEND CMAKE_EXE_LINKER_FLAGS " ${CMAKE_CXX_LINKER_WRAPPER_FLAG}-stack:10000000")
elseif(MINGW OR MSYS OR CYGWIN)
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -Wl,--stack,10000000")
elseif(_CLANG_MSVC_WINDOWS AND "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xGNU")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -Xlinker -stack:20000000")
endif()

#silence duplicate symbol warnings on AIX
if(CMAKE_SYSTEM_NAME MATCHES "AIX")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -bhalt:5 ")
  endif()
endif()

if(CMAKE_SYSTEM MATCHES "OSF1-V")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local -no_implicit_include ")
  endif()
endif()

# Workaround for short jump tables on PA-RISC
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^parisc")
  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlong-calls")
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlong-calls")
  endif()
endif()

# Use 64-bit off_t on 32-bit Linux
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  # ensure 64bit offsets are used for filesystem accesses for 32bit compilation
  add_compile_definitions(_FILE_OFFSET_BITS=64)
endif()

# Workaround for TOC Overflow on ppc64
set(bigTocFlag "")
if(CMAKE_SYSTEM_NAME STREQUAL "AIX" AND
   CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
  set(bigTocFlag "-Wl,-bbigtoc")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
   CMAKE_SYSTEM_PROCESSOR MATCHES "ppc64")
  set(bigTocFlag "-Wl,--no-multi-toc")
endif()
if(bigTocFlag)
  include(CheckCXXLinkerFlag)
  check_cxx_linker_flag(${bigTocFlag} BIG_TOC_FLAG_SUPPORTED)
  if(BIG_TOC_FLAG_SUPPORTED)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${bigTocFlag}")
  endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL SunPro AND
    NOT DEFINED CMAKE_CXX${CMAKE_CXX_STANDARD}_STANDARD_COMPILE_OPTION)
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.13)
    if (NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD EQUAL 98)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
    endif()
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stlport4")
  endif()
endif()

foreach(lang IN ITEMS C CXX)
  # Suppress warnings from PGI compiler.
  if (CMAKE_${lang}_COMPILER_ID STREQUAL "PGI")
    set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} -w")
  endif()
endforeach()

# use the ansi CXX compile flag for building cmake
if (CMAKE_ANSI_CXXFLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")
endif ()

if (CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif ()

# Allow per-translation-unit parallel builds when using MSVC
if(CMAKE_GENERATOR MATCHES "Visual Studio" AND
   (CMAKE_C_COMPILER_ID MATCHES "MSVC|Intel" OR
   CMAKE_CXX_COMPILER_ID MATCHES "MSVC|Intel"))

  set(CMake_MSVC_PARALLEL ON CACHE STRING "\
Enables /MP flag for parallel builds using MSVC. Specify an \
integer value to control the number of threads used (Only \
works on some older versions of Visual Studio). Setting to \
ON lets the toolchain decide how many threads to use. Set to \
OFF to disable /MP completely." )

  if(CMake_MSVC_PARALLEL)
    if(CMake_MSVC_PARALLEL GREATER 0)
      string(APPEND CMAKE_C_FLAGS " /MP${CMake_MSVC_PARALLEL}")
      string(APPEND CMAKE_CXX_FLAGS " /MP${CMake_MSVC_PARALLEL}")
    else()
      string(APPEND CMAKE_C_FLAGS " /MP")
      string(APPEND CMAKE_CXX_FLAGS " /MP")
    endif()
  endif()
endif()

# Get rid of excess -Wunused-but-set-variable on release builds with LCC >= 1.26
foreach(l IN ITEMS C CXX)
  if(CMAKE_${l}_COMPILER_ID STREQUAL "LCC" AND NOT CMAKE_${l}_COMPILER_VERSION VERSION_LESS 1.26)
    foreach(c IN ITEMS MINSIZEREL RELEASE RELWITHDEBINFO)
      string(APPEND "CMAKE_${l}_FLAGS_${c}" " -Wno-unused-but-set-variable")
    endforeach()
  endif()
endforeach()
