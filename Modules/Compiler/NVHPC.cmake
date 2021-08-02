# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__COMPILER_NVHPC)
  return()
endif()
set(__COMPILER_NVHPC 1)

include(Compiler/PGI)

macro(__compiler_nvhpc lang)
  # Logic specific to NVHPC.

  if(CMAKE_${lang}_COMPILER_VERSION VERSION_GREATER_EQUAL 21.07)
    set(CMAKE_DEPFILE_FLAGS_${lang} "-MD -MT <DEP_TARGET> -MF <DEP_FILE>")
    set(CMAKE_${lang}_DEPFILE_FORMAT gcc)
    set(CMAKE_${lang}_DEPENDS_USE_COMPILER TRUE)
  else()
    # Before NVHPC 21.07 the `-MD` flag implicitly
    # implies `-E` and therefore compilation and dependency generation
    # can't occur in the same invocation
    set(CMAKE_${lang}_DEPENDS_EXTRA_COMMANDS "<CMAKE_${lang}_COMPILER> <DEFINES> <INCLUDES> <FLAGS> ${CMAKE_${lang}_COMPILE_OPTIONS_EXPLICIT_LANGUAGE} -M <SOURCE> -MT <OBJECT> -MD<DEP_FILE>")
  endif()

endmacro()
