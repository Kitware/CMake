#
# helper CMakeLists.txt file that can be used to generate input files
# for the Tests/RunCMake/ParseImplicit[Include|Lib]Info tests.
#
# usage:
#  [create a temporary build directory and chdir to it]
#  cmake [-D options] $CMAKE_SRC/Tests/RunCMake/ParseImplicitIncludeInfo/data
#
# where useful -D options include:
#   -DLANGUAGES="C;CXX"    -- list of languages to generate inputs for
#   -DUNAME="Darwin"       -- operating system name (def: CMAKE_SYSTEM_NAME)
#

cmake_minimum_required(VERSION 3.3)
if(POLICY CMP0089)
  cmake_policy(SET CMP0089 NEW)
endif()

set(lngs C CXX)
set(LANGUAGES "${lngs}" CACHE STRING "List of languages to generate inputs for")

project(gen_implicit_include_data ${LANGUAGES})

set(UNAME "${CMAKE_SYSTEM_NAME}" CACHE STRING "System uname")
string(TOLOWER "${UNAME}" UNAME)
message("Generate input for system type: ${UNAME}")

# CMAKE_<LANG>_COMPILER_* variables we save in the resultfile
set(compvars ABI AR ARCHITECTURE_ID EXTERNAL_TOOLCHAIN ID LAUNCHER LOADED
  RANLIB TARGET VERSION VERSION_INTERAL)

foreach(lang IN ITEMS ${LANGUAGES})

  if("${lang}" STREQUAL "C")
    set(file ${CMAKE_ROOT}/Modules/CMakeCCompilerABI.c)
  elseif("${lang}" STREQUAL "CXX")
    set(file ${CMAKE_ROOT}/Modules/CMakeCXXCompilerABI.cpp)
  elseif("${lang}" STREQUAL "CUDA")
    set(file ${CMAKE_ROOT}/Modules/CMakeCUDACompilerABI.cu)
  elseif("${lang}" STREQUAL "Fortran")
    set(file ${CMAKE_ROOT}/Modules/CMakeFortranCompilerABI.F)
  else()
    message(FATAL_ERROR "unknown language ${lang}")
  endif()

  set(resultfile "${CMAKE_BINARY_DIR}/")
  string(APPEND resultfile ${UNAME}-${lang}-${CMAKE_${lang}_COMPILER_ID})
  string(APPEND resultfile -${CMAKE_${lang}_COMPILER_VERSION})
  string(APPEND resultfile .input)
  message("Generate input for language ${lang}")
  message("Input file: ${file}")
  message("Result file: ${resultfile}")

  # replicate logic from CMakeDetermineCompilerABI
  set(outfile "${CMAKE_PLATFORM_INFO_DIR}/test${lang}.out")
  set(CMAKE_FLAGS )
  set(COMPILE_DEFINITIONS )
  set(LINK_OPTIONS )
  if(DEFINED CMAKE_${lang}_VERBOSE_FLAG)
    set(LINK_OPTIONS "${CMAKE_${lang}_VERBOSE_FLAG}")
    set(COMPILE_DEFINITIONS "${CMAKE_${lang}_VERBOSE_FLAG}")
  endif()
  if(DEFINED CMAKE_${lang}_VERBOSE_COMPILE_FLAG)
    set(COMPILE_DEFINITIONS "${CMAKE_${lang}_VERBOSE_COMPILE_FLAG}")
  endif()
  if(DEFINED CMAKE_${lang}_VERBOSE_LINK_FLAG)
    list(APPEND LINK_OPTIONS "${CMAKE_${lang}_VERBOSE_LINK_FLAG}")
  endif()
  if(NOT "x${CMAKE_${lang}_COMPILER_ID}" STREQUAL "xMSVC")
    # Avoid adding our own platform standard libraries for compilers
    # from which we might detect implicit link libraries.
    list(APPEND CMAKE_FLAGS "-DCMAKE_${lang}_STANDARD_LIBRARIES=")
  endif()
  list(JOIN LINK_OPTIONS " " LINK_OPTIONS)
  list(APPEND CMAKE_FLAGS "-DEXE_LINKER_FLAGS=${LINK_OPTIONS}")

  try_compile(rv ${CMAKE_BINARY_DIR} ${file}
    CMAKE_FLAGS ${CMAKE_FLAGS}
    COMPILE_DEFINITIONS ${COMPILE_DEFINITIONS}
    CMAKE_FLAGS ${CMAKE_FLAGS}
    OUTPUT_VARIABLE output
    COPY_FILE "${outfile}"
    COPY_FILE_ERROR copy_error)

  if(NOT rv)
    message(FATAL_ERROR "${lang} compile failed!!")
  endif()

  set(result "CMAKE_LANG=${lang}\n")
  list(APPEND result "CMAKE_LINKER=${CMAKE_LINKER}\n")
  foreach(var IN ITEMS ${compvars})
    list(APPEND result
      "CMAKE_${lang}_COMPILER_${var}=${CMAKE_${lang}_COMPILER_${var}}\n")
  endforeach()

  file(WRITE ${resultfile} ${result} ${output})
endforeach()
