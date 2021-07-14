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

set(_CMAKE_TOOL_VARS "")

# if it's the MS C/CXX compiler, search for link
if(("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_SIMULATE_ID}" STREQUAL "xMSVC" AND
   ("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC"
    OR NOT "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xClang"))
   OR "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xMSVC"
   OR (CMAKE_HOST_WIN32 AND "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xPGI")
   OR (CMAKE_HOST_WIN32 AND "x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xNVIDIA")
   OR (CMAKE_HOST_WIN32 AND "x${_CMAKE_PROCESSING_LANGUAGE}" STREQUAL "xISPC")
   OR (CMAKE_GENERATOR MATCHES "Visual Studio"
       AND NOT CMAKE_VS_PLATFORM_NAME STREQUAL "Tegra-Android"))

  # Start with the canonical names.
  set(_CMAKE_LINKER_NAMES "link")
  set(_CMAKE_AR_NAMES "lib")
  set(_CMAKE_MT_NAMES "mt")

  # Prepend toolchain-specific names.
  if("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL "xClang")
    set(_CMAKE_NM_NAMES "llvm-nm" "nm")
    list(PREPEND _CMAKE_AR_NAMES "llvm-lib")
    list(PREPEND _CMAKE_MT_NAMES "llvm-mt")
    list(PREPEND _CMAKE_LINKER_NAMES "lld-link")
    list(APPEND _CMAKE_TOOL_VARS NM)
  elseif("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" MATCHES "^xIntel")
    list(PREPEND _CMAKE_AR_NAMES "xilib")
    list(PREPEND _CMAKE_LINKER_NAMES "xilink")
  endif()

  list(APPEND _CMAKE_TOOL_VARS LINKER MT AR)

elseif("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" MATCHES "^x(Open)?Watcom$")
  set(_CMAKE_LINKER_NAMES "wlink")
  set(_CMAKE_AR_NAMES "wlib")
  list(APPEND _CMAKE_TOOL_VARS LINKER AR)

# in all other cases search for ar, ranlib, etc.
else()
  if(CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN)
    set(_CMAKE_TOOLCHAIN_LOCATION ${_CMAKE_TOOLCHAIN_LOCATION} ${CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN}/bin)
  endif()
  if(CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN)
    set(_CMAKE_TOOLCHAIN_LOCATION ${_CMAKE_TOOLCHAIN_LOCATION} ${CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN}/bin)
  endif()

  # Start with the canonical names.
  set(_CMAKE_AR_NAMES "ar")
  set(_CMAKE_RANLIB_NAMES "ranlib")
  set(_CMAKE_STRIP_NAMES "strip")
  set(_CMAKE_LINKER_NAMES "ld")
  set(_CMAKE_NM_NAMES "nm")
  set(_CMAKE_OBJDUMP_NAMES "objdump")
  set(_CMAKE_OBJCOPY_NAMES "objcopy")
  set(_CMAKE_READELF_NAMES "readelf")
  set(_CMAKE_DLLTOOL_NAMES "dlltool")
  set(_CMAKE_ADDR2LINE_NAMES "addr2line")

  # Prepend toolchain-specific names.
  if("${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL Clang)
    if("x${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_SIMULATE_ID}" STREQUAL "xMSVC")
      set(_CMAKE_LINKER_NAMES "lld-link")
    else()
      set(_CMAKE_LINKER_NAMES "ld.lld")
    endif()
    list(PREPEND _CMAKE_AR_NAMES "llvm-ar")
    list(PREPEND _CMAKE_RANLIB_NAMES "llvm-ranlib")
    list(PREPEND _CMAKE_STRIP_NAMES "llvm-strip")
    list(PREPEND _CMAKE_NM_NAMES "llvm-nm")
    if("${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_VERSION}" VERSION_GREATER_EQUAL 9)
      # llvm-objdump versions prior to 9 did not support everything we need.
      list(PREPEND _CMAKE_OBJDUMP_NAMES "llvm-objdump")
    endif()
    list(PREPEND _CMAKE_OBJCOPY_NAMES "llvm-objcopy")
    list(PREPEND _CMAKE_READELF_NAMES "llvm-readelf")
    list(PREPEND _CMAKE_DLLTOOL_NAMES "llvm-dlltool")
    list(PREPEND _CMAKE_ADDR2LINE_NAMES "llvm-addr2line")
  elseif("${CMAKE_${_CMAKE_PROCESSING_LANGUAGE}_COMPILER_ID}" STREQUAL ARMClang)
    list(PREPEND _CMAKE_AR_NAMES "armar")
    list(PREPEND _CMAKE_LINKER_NAMES "armlink")
  endif()

  list(APPEND _CMAKE_TOOL_VARS AR RANLIB STRIP LINKER NM OBJDUMP OBJCOPY READELF DLLTOOL ADDR2LINE)
endif()

foreach(_CMAKE_TOOL IN LISTS _CMAKE_TOOL_VARS)
  # Build the final list of prefixed/suffixed names.
  set(_CMAKE_${_CMAKE_TOOL}_FIND_NAMES "")
  foreach(_CMAKE_TOOL_NAME IN LISTS _CMAKE_${_CMAKE_TOOL}_NAMES)
    list(APPEND _CMAKE_${_CMAKE_TOOL}_FIND_NAMES
      ${_CMAKE_TOOLCHAIN_PREFIX}${_CMAKE_TOOL_NAME}${_CMAKE_TOOLCHAIN_SUFFIX}
      ${_CMAKE_TOOLCHAIN_PREFIX}${_CMAKE_TOOL_NAME}
      ${_CMAKE_TOOL_NAME}${_CMAKE_TOOLCHAIN_SUFFIX}
      ${_CMAKE_TOOL_NAME}
      )
  endforeach()
  list(REMOVE_DUPLICATES _CMAKE_${_CMAKE_TOOL}_FIND_NAMES)

  find_program(CMAKE_${_CMAKE_TOOL} NAMES ${_CMAKE_${_CMAKE_TOOL}_FIND_NAMES} HINTS ${_CMAKE_TOOLCHAIN_LOCATION})
  unset(_CMAKE_${_CMAKE_TOOL}_FIND_NAMES)
endforeach()

if(NOT CMAKE_RANLIB)
    set(CMAKE_RANLIB : CACHE INTERNAL "noop for ranlib")
endif()


if(CMAKE_PLATFORM_HAS_INSTALLNAME)
  find_program(CMAKE_INSTALL_NAME_TOOL NAMES ${_CMAKE_TOOLCHAIN_PREFIX}install_name_tool HINTS ${_CMAKE_TOOLCHAIN_LOCATION})

  if(NOT CMAKE_INSTALL_NAME_TOOL)
    message(FATAL_ERROR "Could not find install_name_tool, please check your installation.")
  endif()

  list(APPEND _CMAKE_TOOL_VARS INSTALL_NAME_TOOL)
endif()

# Mark any tool cache entries as advanced.
foreach(_CMAKE_TOOL IN LISTS _CMAKE_TOOL_VARS)
  get_property(_CMAKE_TOOL_CACHED CACHE CMAKE_${_CMAKE_TOOL} PROPERTY TYPE)
  if(_CMAKE_TOOL_CACHED)
    mark_as_advanced(CMAKE_${_CMAKE_TOOL})
  endif()
  unset(_CMAKE_${_CMAKE_TOOL}_NAMES)
endforeach()
unset(_CMAKE_TOOL_VARS)
unset(_CMAKE_TOOL_CACHED)
unset(_CMAKE_TOOL_NAME)
unset(_CMAKE_TOOL)
