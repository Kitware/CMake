# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__AIX_COMPILER_XL)
  return()
endif()
set(__AIX_COMPILER_XL 1)

macro(__aix_compiler_xl lang)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG "-Wl,-blibpath:")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG_SEP ":")
  string(APPEND CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS " -Wl,-bnoipath")
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-Wl,-bexpall") # CMP0065 old behavior
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS " ")
  set(CMAKE_SHARED_MODULE_${lang}_FLAGS  " ")

  set(CMAKE_${lang}_LINK_FLAGS "-Wl,-bnoipath")

  # Construct the export list ourselves to pass only the object files so
  # that we export only the symbols actually provided by the sources.
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "\"${CMAKE_ROOT}/Modules/Platform/AIX/ExportImportList\" -o <OBJECT_DIR>/objects.exp <OBJECTS>"
    "<CMAKE_${lang}_COMPILER> <CMAKE_SHARED_LIBRARY_${lang}_FLAGS> -Wl,-bE:<OBJECT_DIR>/objects.exp <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
    )

  set(CMAKE_${lang}_LINK_EXECUTABLE_WITH_EXPORTS
    "\"${CMAKE_ROOT}/Modules/Platform/AIX/ExportImportList\" -o <TARGET_IMPLIB> -l . <OBJECTS>"
    "<CMAKE_${lang}_COMPILER> <FLAGS> <CMAKE_${lang}_LINK_FLAGS> -Wl,-bE:<TARGET_IMPLIB> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endmacro()
