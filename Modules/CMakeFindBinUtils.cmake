# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# search for additional tools required for C/C++ (and other languages ?)
#
# If the internal cmake variable _CMAKE_TOOLCHAIN_PREFIX is set, this is used
# as prefix for the tools (e.g. arm-elf-gcc etc.)
# If the cmake variable _CMAKE_TOOLCHAIN_LOCATION is set, the compiler is
# searched only there. The other tools are at first searched there, then
# also in the default locations.
#
# Sets the following variables:
#   CMAKE_AR
#   CMAKE_RANLIB
#   CMAKE_LINKER
#   CMAKE_MT
#   CMAKE_STRIP
#   CMAKE_INSTALL_NAME_TOOL

# on UNIX, cygwin and mingw

# Resolve full path of CMAKE_TOOL from user-defined name and SEARCH_PATH.
function(__resolve_tool_path CMAKE_TOOL SEARCH_PATH DOCSTRING)

  if(${CMAKE_TOOL})
    # We only get here if CMAKE_TOOL was
    # specified using -D or a pre-made CMakeCache.txt (e.g. via ctest)
    # or set in CMAKE_TOOLCHAIN_FILE.

    get_filename_component(_CMAKE_USER_TOOL_PATH "${${CMAKE_TOOL}}" DIRECTORY)
    # Is CMAKE_TOOL a user-defined name instead of a full path?
    if(NOT _CMAKE_USER_TOOL_PATH)

      # Find CMAKE_TOOL in the SEARCH_PATH directory by user-defined name.
      find_program(_CMAKE_TOOL_WITH_PATH NAMES ${${CMAKE_TOOL}} HINTS ${SEARCH_PATH})
      if(_CMAKE_TOOL_WITH_PATH)

        # Overwrite CMAKE_TOOL with full path found in SEARCH_PATH.
        set(${CMAKE_TOOL} ${_CMAKE_TOOL_WITH_PATH} PARENT_SCOPE)

        get_property(_CMAKE_TOOL_CACHED CACHE ${CMAKE_TOOL} PROPERTY TYPE)
        # If CMAKE_TOOL is present in the CMake Cache, then overwrit it as well.
        if(_CMAKE_TOOL_CACHED)
          set(${CMAKE_TOOL} "${_CMAKE_TOOL_WITH_PATH}" CACHE STRING ${DOCSTRING} FORCE)
        endif()

      endif()
      unset(_CMAKE_TOOL_WITH_PATH CACHE)

    endif()

  endif()

endfunction()

__resolve_tool_path(CMAKE_LINKER "${_CMAKE_TOOLCHAIN_LOCATION}" "Default Linker")
__resolve_tool_path(CMAKE_MT     "${_CMAKE_TOOLCHAIN_LOCATION}" "Default Manifest Tool")

function(__get_compiler_component CMAKE_TOOL NAME)
  get_property(_CMAKE_TOOL_CACHED CACHE ${CMAKE_TOOL} PROPERTY TYPE)
  # If CMAKE_TOOL is present in the CMake Cache, return
  if(_CMAKE_TOOL_CACHED)
    return()
  endif()

  cmake_parse_arguments(_COMPILER_COMP_ARGS "" "DOC" "HINTS;NAMES" ${ARGN})

  set(_LOCATION_FROM_COMPILER )
  set(_NAME_FROM_COMPILER )

  if (NOT DEFINED ${CMAKE_TOOL})
    if("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xGNU" OR
       "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xClang")
      execute_process(
        COMMAND ${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER} -print-prog-name=${NAME}
        RESULT_VARIABLE _CMAKE_TOOL_PROG_NAME_RESULT
        OUTPUT_VARIABLE _CMAKE_TOOL_PROG_NAME_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if (_CMAKE_TOOL_PROG_NAME_RESULT STREQUAL "0" AND IS_ABSOLUTE "${_CMAKE_TOOL_PROG_NAME_OUTPUT}")
        get_filename_component(_LOCATION_FROM_COMPILER "${_CMAKE_TOOL_PROG_NAME_OUTPUT}" DIRECTORY)
        get_filename_component(_NAME_FROM_COMPILER "${_CMAKE_TOOL_PROG_NAME_OUTPUT}" NAME)
      endif()
    endif()
  endif()

  if (NOT _COMPILER_COMP_ARGS_DOC)
    set(_COMPILER_COMP_ARGS_DOC "Path to ${NAME} program")
  endif()
  find_program(${CMAKE_TOOL}
    NAMES ${_NAME_FROM_COMPILER} ${_COMPILER_COMP_ARGS_NAMES}
    HINTS ${_LOCATION_FROM_COMPILER} ${_COMPILER_COMP_ARGS_HINTS}
    DOC "${_COMPILER_COMP_ARGS_DOC}"
  )
endfunction()

set(_CMAKE_TOOL_VARS "")

# if it's the MS C/CXX compiler, search for link
if(("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_SIMULATE_ID}" STREQUAL "xMSVC" AND
   ("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC"
    OR NOT "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xClang"))
   OR "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xMSVC"
   OR (CMAKE_HOST_WIN32 AND "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xPGI")
   OR (CMAKE_HOST_WIN32 AND "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xNVIDIA")
   OR (CMAKE_GENERATOR MATCHES "Visual Studio"
       AND NOT CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android"))

  if("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xClang")
    find_program(CMAKE_NM NAMES ${_CMAKE_TOOLCHAIN_PREFIX}nm llvm-nm HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
    set(_CMAKE_ADDITIONAL_LINKER_NAMES "lld-link")
  endif()

  find_program(CMAKE_LINKER NAMES ${_CMAKE_ADDITIONAL_LINKER_NAMES} link HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  find_program(CMAKE_MT     NAMES mt   HINTS ${_CMAKE_TOOLCHAIN_LOCATION})

  list(APPEND _CMAKE_TOOL_VARS LINKER MT)

# in all other cases search for ar, ranlib, etc.
else()
  if(CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN)
    set(_CMAKE_TOOLCHAIN_LOCATION ${_CMAKE_TOOLCHAIN_LOCATION} ${CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN}/bin)
  endif()
  if(CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN)
    set(_CMAKE_TOOLCHAIN_LOCATION ${_CMAKE_TOOLCHAIN_LOCATION} ${CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN}/bin)
  endif()

  if("${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL Clang)
    set(_CMAKE_ADDITIONAL_AR_NAMES "llvm-ar")
    set(_CMAKE_ADDITIONAL_RANLIB_NAMES "llvm-ranlib")
    set(_CMAKE_ADDITIONAL_STRIP_NAMES "llvm-strip")
    set(_CMAKE_ADDITIONAL_LINKER_NAMES "ld.lld")
    set(_CMAKE_ADDITIONAL_NM_NAMES "llvm-nm")
    set(_CMAKE_ADDITIONAL_OBJDUMP_NAMES "llvm-objdump")
    set(_CMAKE_ADDITIONAL_OBJCOPY_NAMES "llvm-objcopy")
    set(_CMAKE_ADDITIONAL_READELF_NAMES "llvm-readelf")
    set(_CMAKE_ADDITIONAL_DLLTOOL_NAMES "llvm-dlltool")
    set(_CMAKE_ADDITIONAL_ADDR2LINE_NAMES "llvm-addr2line")
  endif()

  __get_compiler_component(CMAKE_AR ar NAMES ${_CMAKE_TOOLCHAIN_PREFIX}ar${_CMAKE_TOOLCHAIN_SUFFIX} ${_CMAKE_ADDITIONAL_AR_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})

  __get_compiler_component(CMAKE_RANLIB ranlib NAMES ${_CMAKE_TOOLCHAIN_PREFIX}ranlib ${_CMAKE_ADDITIONAL_RANLIB_NAMES}
                                               HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  if(NOT CMAKE_RANLIB)
    set(CMAKE_RANLIB : CACHE INTERNAL "noop for ranlib")
  endif()


  __get_compiler_component(CMAKE_STRIP strip NAMES ${_CMAKE_TOOLCHAIN_PREFIX}strip${_CMAKE_TOOLCHAIN_SUFFIX} ${_CMAKE_ADDITIONAL_STRIP_NAMES}
                                             HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_LINKER ld NAMES ${_CMAKE_TOOLCHAIN_PREFIX}ld ${_CMAKE_ADDITIONAL_LINKER_NAMES}
                                           HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_NM nm NAMES ${_CMAKE_TOOLCHAIN_PREFIX}nm ${_CMAKE_ADDITIONAL_NM_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_OBJDUMP objdump NAMES ${_CMAKE_TOOLCHAIN_PREFIX}objdump ${_CMAKE_ADDITIONAL_OBJDUMP_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_OBJCOPY objcopy NAMES ${_CMAKE_TOOLCHAIN_PREFIX}objcopy ${_CMAKE_ADDITIONAL_OBJCOPY_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_READELF readelf NAMES ${_CMAKE_TOOLCHAIN_PREFIX}readelf ${_CMAKE_ADDITIONAL_READELF_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_DLLTOOL dlltool NAMES ${_CMAKE_TOOLCHAIN_PREFIX}dlltool ${_CMAKE_ADDITIONAL_DLLTOOL_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  __get_compiler_component(CMAKE_ADDR2LINE addr2line NAMES ${_CMAKE_TOOLCHAIN_PREFIX}addr2line ${_CMAKE_ADDITIONAL_ADDR2LINE_NAMES}
                                       HINTS ${_CMAKE_TOOLCHAIN_LOCATION})

  list(APPEND _CMAKE_TOOL_VARS AR RANLIB STRIP LINKER NM OBJDUMP OBJCOPY READELF DLLTOOL ADDR2LINE)
endif()

if(CMAKE_PLATFORM_HAS_INSTALLNAME)
  __get_compiler_component(CMAKE_INSTALL_NAME_TOOL install_name_tool NAMES ${_CMAKE_TOOLCHAIN_PREFIX}install_name_tool HINTS ${_CMAKE_TOOLCHAIN_LOCATION})

  if(NOT CMAKE_INSTALL_NAME_TOOL)
    message(FATAL_ERROR "Could not find install_name_tool, please check your installation.")
  endif()

  list(APPEND _CMAKE_TOOL_VARS INSTALL_NAME_TOOL)
endif()

# Mark any tool cache entries as advanced.
foreach(var IN LISTS _CMAKE_TOOL_VARS)
  get_property(_CMAKE_TOOL_CACHED CACHE CMAKE_${var} PROPERTY TYPE)
  if(_CMAKE_TOOL_CACHED)
    mark_as_advanced(CMAKE_${var})
  endif()
  unset(_CMAKE_ADDITIONAL_${var}_NAMES)
endforeach()
unset(_CMAKE_TOOL_VARS)
unset(_CMAKE_TOOL_CACHED)
