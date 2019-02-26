# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This module is shared by multiple languages; use include blocker.
if(__AIX_COMPILER_XL)
  return()
endif()
set(__AIX_COMPILER_XL 1)

#
# By default, runtime linking is enabled. All shared objects specified on the command line
# will be listed, even if there are no symbols referenced, in the output file.
string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " -Wl,-brtl")
string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " -Wl,-brtl")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -Wl,-brtl")


macro(__aix_compiler_xl lang)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG "-Wl,-blibpath:")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG_SEP ":")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-G -Wl,-bnoipath")  # -shared
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-Wl,-bexpall")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS " ")
  set(CMAKE_SHARED_MODULE_${lang}_FLAGS  " ")

  set(CMAKE_${lang}_LINK_FLAGS "-Wl,-bnoipath")

  # Find the CreateExportList program that comes with this toolchain.
  find_program(CMAKE_XL_CreateExportList
    NAMES CreateExportList
    DOC "IBM XL CreateExportList tool"
    )

  # CMAKE_XL_CreateExportList is part of the AIX XL compilers but not the linux ones.
  # If we found the tool, we'll use it to create exports, otherwise stick with the regular
  # create shared library compile line.
  if (CMAKE_XL_CreateExportList)
    # The compiler front-end passes all object files, archive files, and shared
    # library files named on the command line to CreateExportList to create a
    # list of all symbols to be exported from the shared library.  This causes
    # all archive members to be copied into the shared library whether they are
    # needed or not.  Instead we run the tool ourselves to pass only the object
    # files so that we export only the symbols actually provided by the sources.
    set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
      "${CMAKE_XL_CreateExportList} <OBJECT_DIR>/objects.exp <OBJECTS>"
      "<CMAKE_${lang}_COMPILER> <CMAKE_SHARED_LIBRARY_${lang}_FLAGS> -Wl,-bE:<OBJECT_DIR>/objects.exp <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
      )
  endif()
endmacro()
