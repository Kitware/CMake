# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Function to identify the linker.  This is used internally by CMake and should
# not be included by user code.
# If successful, sets CMAKE_<lang>_COMPILER_LINKER_ID and
# CMAKE_<lang>_COMPILER_LINKER_VERSION

cmake_policy(PUSH)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0054 NEW)

function(cmake_determine_linker_id lang linker)
  if (NOT linker)
    # linker was not identified
    unset(CMAKE_${lang}_COMPILER_LINKER_ID PARENT_SCOPE)
    unset(CMAKE_${lang}_COMPILER_LINKER_VERSION PARENT_SCOPE)
    unset(CMAKE_${lang}_COMPILER_LINKER_FRONTEND_VARIANT PARENT_SCOPE)
    return()
  endif()

  if (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR linker MATCHES "lld$")
    set(flags "--version")
  else()
    set(flags "-v")
  endif()
  execute_process(COMMAND "${linker}" ${flags}
                  OUTPUT_VARIABLE linker_desc
                  ERROR_VARIABLE linker_desc
                  OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_STRIP_TRAILING_WHITESPACE)

  set(linker_frontend)
  set(linker_version)

  # Compute the linker ID
  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND linker_desc MATCHES "@\\(#\\)PROGRAM:ld")
    set(linker_id "AppleClang")
    set(linker_frontend "GNU")
  elseif (linker_desc MATCHES "mold \\(sold\\)")
    set(linker_id "MOLD")
    set(linker_frontend "GNU")
  elseif (linker_desc MATCHES "mold")
    set(linker_id "MOLD")
    set(linker_frontend "GNU")
  elseif (linker_desc MATCHES "LLD")
    set(linker_id "LLD")
    set(linker_frontend "GNU")
    if (WIN32 AND NOT linker_desc MATCHES "compatible with GNU")
      set (linker_frontend "MSVC")
    endif()
  elseif (linker_desc MATCHES "GNU ld")
    set(linker_id "GNU")
    set(linker_frontend "GNU")
  elseif (linker_desc MATCHES "GNU gold")
    set(linker_id "GNUgold")
    set(linker_frontend "GNU")
  elseif (linker_desc MATCHES "Microsoft \\(R\\) Incremental Linker")
    set(linker_id "MSVC")
    set(linker_frontend "MSVC")
  else()
    # unknown linker
    set(linker_id "UNKNOWN")
  endif()

  # Get linker version
  if (linker_id STREQUAL "AppleClang")
    string(REGEX REPLACE ".+PROJECT:[a-z0-9]+-([0-9.]+).+" "\\1" linker_version "${linker_desc}")
  elseif (linker_id MATCHES "MOLD|SOLD")
    string(REGEX REPLACE "^mold (\\(sold\\) )?([0-9.]+).+" "\\2" linker_version "${linker_desc}")
  elseif (linker_id STREQUAL "LLD")
    string(REGEX REPLACE ".*LLD ([0-9.]+).*" "\\1" linker_version "${linker_desc}")
  elseif (linker_id MATCHES "(GNU|GOLD)")
    string(REGEX REPLACE "^GNU [^ ]+ \\([^)]+\\) ([0-9.]+).*" "\\1" linker_version "${linker_desc}")
  elseif (linker_id STREQUAL "MSVC")
    string(REGEX REPLACE ".+Linker Version ([0-9.]+).+" "\\1" linker_version "${linker_desc}")
  endif()

  set(CMAKE_${lang}_COMPILER_LINKER_ID "${linker_id}" PARENT_SCOPE)
  if (linker_frontend)
    set(CMAKE_${lang}_COMPILER_LINKER_FRONTEND_VARIANT "${linker_frontend}" PARENT_SCOPE)
  else()
    unset(CMAKE_${lang}_COMPILER_LINKER_FRONTEND_VARIANT PARENT_SCOPE)
  endif()
  if (linker_version)
    set(CMAKE_${lang}_COMPILER_LINKER_VERSION "${linker_version}" PARENT_SCOPE)
  else()
    unset(CMAKE_${lang}_COMPILER_LINKER_VERSION PARENT_SCOPE)
  endif()
endfunction()

cmake_policy(POP)
