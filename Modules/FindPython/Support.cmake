# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#
# This file is a "template" file used by various FindPython modules.
#

cmake_policy (GET CMP0094 _${_PYTHON_PREFIX}_LOOKUP_POLICY)

cmake_policy (VERSION 3.7)

if (_${_PYTHON_PREFIX}_LOOKUP_POLICY)
  cmake_policy (SET CMP0094 ${_${_PYTHON_PREFIX}_LOOKUP_POLICY})
endif()

#
# Initial configuration
#
if (NOT DEFINED _PYTHON_PREFIX)
  message (FATAL_ERROR "FindPython: INTERNAL ERROR")
endif()
if (NOT DEFINED _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
  message (FATAL_ERROR "FindPython: INTERNAL ERROR")
endif()
if (_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR EQUAL 3)
  set(_${_PYTHON_PREFIX}_VERSIONS 3.9 3.8 3.7 3.6 3.5 3.4 3.3 3.2 3.1 3.0)
elseif (_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR EQUAL 2)
  set(_${_PYTHON_PREFIX}_VERSIONS 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0)
else()
  message (FATAL_ERROR "FindPython: INTERNAL ERROR")
endif()

get_property(_${_PYTHON_PREFIX}_CMAKE_ROLE GLOBAL PROPERTY CMAKE_ROLE)


#
# helper commands
#
macro (_PYTHON_DISPLAY_FAILURE _PYTHON_MSG)
  if (${_PYTHON_PREFIX}_FIND_REQUIRED)
    message (FATAL_ERROR "${_PYTHON_MSG}")
  else()
    if (NOT ${_PYTHON_PREFIX}_FIND_QUIETLY)
      message(STATUS "${_PYTHON_MSG}")
    endif ()
  endif()

  set (${_PYTHON_PREFIX}_FOUND FALSE)
  string (TOUPPER "${_PYTHON_PREFIX}" _${_PYTHON_PREFIX}_UPPER_PREFIX)
  set (${_PYTHON_UPPER_PREFIX}_FOUND FALSE)
  return()
endmacro()


function (_PYTHON_MARK_AS_INTERNAL)
  foreach (var IN LISTS ARGV)
    if (DEFINED CACHE{${var}})
      set_property (CACHE ${var} PROPERTY TYPE INTERNAL)
    endif()
  endforeach()
endfunction()


macro (_PYTHON_SELECT_LIBRARY_CONFIGURATIONS _PYTHON_BASENAME)
  if(NOT DEFINED ${_PYTHON_BASENAME}_LIBRARY_RELEASE)
    set(${_PYTHON_BASENAME}_LIBRARY_RELEASE "${_PYTHON_BASENAME}_LIBRARY_RELEASE-NOTFOUND")
  endif()
  if(NOT DEFINED ${_PYTHON_BASENAME}_LIBRARY_DEBUG)
    set(${_PYTHON_BASENAME}_LIBRARY_DEBUG "${_PYTHON_BASENAME}_LIBRARY_DEBUG-NOTFOUND")
  endif()

  get_property(_PYTHON_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if (${_PYTHON_BASENAME}_LIBRARY_DEBUG AND ${_PYTHON_BASENAME}_LIBRARY_RELEASE AND
      NOT ${_PYTHON_BASENAME}_LIBRARY_DEBUG STREQUAL ${_PYTHON_BASENAME}_LIBRARY_RELEASE AND
      (_PYTHON_isMultiConfig OR CMAKE_BUILD_TYPE))
    # if the generator is multi-config or if CMAKE_BUILD_TYPE is set for
    # single-config generators, set optimized and debug libraries
    set (${_PYTHON_BASENAME}_LIBRARY "")
    foreach (_PYTHON_libname IN LISTS ${_PYTHON_BASENAME}_LIBRARY_RELEASE )
      list( APPEND ${_PYTHON_BASENAME}_LIBRARY optimized "${_PYTHON_libname}" )
    endforeach()
    foreach (_PYTHON_libname IN LISTS ${_PYTHON_BASENAME}_LIBRARY_DEBUG )
      list( APPEND ${_PYTHON_BASENAME}_LIBRARY debug "${_PYTHON_libname}" )
    endforeach()
  elseif (${_PYTHON_BASENAME}_LIBRARY_RELEASE)
    set (${_PYTHON_BASENAME}_LIBRARY "${${_PYTHON_BASENAME}_LIBRARY_RELEASE}")
  elseif (${_PYTHON_BASENAME}_LIBRARY_DEBUG)
    set (${_PYTHON_BASENAME}_LIBRARY "${${_PYTHON_BASENAME}_LIBRARY_DEBUG}")
  else()
    set (${_PYTHON_BASENAME}_LIBRARY "${_PYTHON_BASENAME}_LIBRARY-NOTFOUND")
  endif()

  set (${_PYTHON_BASENAME}_LIBRARIES "${${_PYTHON_BASENAME}_LIBRARY}")
endmacro()


macro (_PYTHON_FIND_FRAMEWORKS)
  set (${_PYTHON_PREFIX}_FRAMEWORKS)
  if (CMAKE_HOST_APPLE OR APPLE)
    file(TO_CMAKE_PATH "$ENV{CMAKE_FRAMEWORK_PATH}" _pff_CMAKE_FRAMEWORK_PATH)
    set (_pff_frameworks ${CMAKE_FRAMEWORK_PATH}
                    ${_pff_CMAKE_FRAMEWORK_PATH}
                    ~/Library/Frameworks
                    /usr/local/Frameworks
                    ${CMAKE_SYSTEM_FRAMEWORK_PATH})
    list (REMOVE_DUPLICATES _pff_frameworks)
    foreach (_pff_framework IN LISTS _pff_frameworks)
      if (EXISTS ${_pff_framework}/Python${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}.framework)
        list (APPEND ${_PYTHON_PREFIX}_FRAMEWORKS ${_pff_framework}/Python${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}.framework)
      endif()
      if (EXISTS ${_pff_framework}/Python.framework)
        list (APPEND ${_PYTHON_PREFIX}_FRAMEWORKS ${_pff_framework}/Python.framework)
      endif()
    endforeach()
    unset (_pff_frameworks)
    unset (_pff_framework)
  endif()
endmacro()

function (_PYTHON_GET_FRAMEWORKS _PYTHON_PGF_FRAMEWORK_PATHS _PYTHON_VERSION)
  set (_PYTHON_FRAMEWORK_PATHS)
  foreach (_PYTHON_FRAMEWORK IN LISTS ${_PYTHON_PREFIX}_FRAMEWORKS)
    list (APPEND _PYTHON_FRAMEWORK_PATHS
          "${_PYTHON_FRAMEWORK}/Versions/${_PYTHON_VERSION}")
  endforeach()
  set (${_PYTHON_PGF_FRAMEWORK_PATHS} ${_PYTHON_FRAMEWORK_PATHS} PARENT_SCOPE)
endfunction()

function (_PYTHON_GET_REGISTRIES _PYTHON_PGR_REGISTRY_PATHS _PYTHON_VERSION)
  string (REPLACE "." "" _PYTHON_VERSION_NO_DOTS ${_PYTHON_VERSION})
  set (${_PYTHON_PGR_REGISTRY_PATHS}
       [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_PYTHON_VERSION}-${_${_PYTHON_PREFIX}_ARCH}\\InstallPath]
       [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_PYTHON_VERSION}-${_${_PYTHON_PREFIX}_ARCH2}\\InstallPath]
       [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_PYTHON_VERSION}\\InstallPath]
       [HKEY_CURRENT_USER\\SOFTWARE\\Python\\ContinuumAnalytics\\Anaconda${_PYTHON_VERSION_NO_DOTS}-${_${_PYTHON_PREFIX}_ARCH}\\InstallPath]
       [HKEY_CURRENT_USER\\SOFTWARE\\Python\\ContinuumAnalytics\\Anaconda${_PYTHON_VERSION_NO_DOTS}-${_${_PYTHON_PREFIX}_ARCH2}\\InstallPath]
       [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_PYTHON_VERSION}-${_${_PYTHON_PREFIX}_ARCH}\\InstallPath]
       [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_PYTHON_VERSION}-${_${_PYTHON_PREFIX}_ARCH2}\\InstallPath]
       [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_PYTHON_VERSION}\\InstallPath]
       [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\ContinuumAnalytics\\Anaconda${_PYTHON_VERSION_NO_DOTS}-${_${_PYTHON_PREFIX}_ARCH}\\InstallPath]
       [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\ContinuumAnalytics\\Anaconda${_PYTHON_VERSION_NO_DOTS}-${_${_PYTHON_PREFIX}_ARCH2}\\InstallPath]
       PARENT_SCOPE)
endfunction()


function (_PYTHON_GET_ABIFLAGS _PGABIFLAGS)
  set (abiflags)
  list (GET _${_PYTHON_PREFIX}_FIND_ABI 0 pydebug)
  list (GET _${_PYTHON_PREFIX}_FIND_ABI 1 pymalloc)
  list (GET _${_PYTHON_PREFIX}_FIND_ABI 2 unicode)

  if (pymalloc STREQUAL "ANY" AND unicode STREQUAL "ANY")
    set (abiflags "mu" "m" "u" "")
  elseif (pymalloc STREQUAL "ANY" AND unicode STREQUAL "ON")
    set (abiflags "mu" "u")
  elseif (pymalloc STREQUAL "ANY" AND unicode STREQUAL "OFF")
    set (abiflags "m" "")
  elseif (pymalloc STREQUAL "ON" AND unicode STREQUAL "ANY")
    set (abiflags "mu" "m")
  elseif (pymalloc STREQUAL "ON" AND unicode STREQUAL "ON")
    set (abiflags "mu")
  elseif (pymalloc STREQUAL "ON" AND unicode STREQUAL "OFF")
    set (abiflags "m")
  elseif (pymalloc STREQUAL "ON" AND unicode STREQUAL "ANY")
    set (abiflags "u" "")
  elseif (pymalloc STREQUAL "OFF" AND unicode STREQUAL "ON")
    set (abiflags "u")
  endif()

  if (pydebug STREQUAL "ON")
    if (abiflags)
      list (TRANSFORM abiflags PREPEND "d")
    else()
      set (abiflags "d")
    endif()
  elseif (pydebug STREQUAL "ANY")
    if (abiflags)
      set (flags "${abiflags}")
      list (TRANSFORM flags PREPEND "d")
      list (APPEND abiflags "${flags}")
    else()
      set (abiflags "" "d")
    endif()
  endif()

  set (${_PGABIFLAGS} "${abiflags}" PARENT_SCOPE)
endfunction()

function (_PYTHON_GET_PATH_SUFFIXES _PYTHON_PGPS_PATH_SUFFIXES)
  cmake_parse_arguments (PARSE_ARGV 1 _PGPS "LIBRARY;INCLUDE" "VERSION" "")

  if (DEFINED _${_PYTHON_PREFIX}_ABIFLAGS)
    set (abi "${_${_PYTHON_PREFIX}_ABIFLAGS}")
  else()
    set (abi "mu" "m" "u" "")
  endif()

  set (path_suffixes)
  if (_PGPS_LIBRARY)
    if (CMAKE_LIBRARY_ARCHITECTURE)
      list (APPEND path_suffixes lib/${CMAKE_LIBRARY_ARCHITECTURE})
    endif()
    list (APPEND path_suffixes lib libs)

    if (CMAKE_LIBRARY_ARCHITECTURE)
      set (suffixes "${abi}")
      if (suffixes)
        list (TRANSFORM suffixes PREPEND "lib/python${_PGPS_VERSION}/config-${_PGPS_VERSION}")
        list (TRANSFORM suffixes APPEND "-${CMAKE_LIBRARY_ARCHITECTURE}")
      else()
        set (suffixes "lib/python${_PGPS_VERSION}/config-${_PGPS_VERSION}-${CMAKE_LIBRARY_ARCHITECTURE}")
      endif()
      list (APPEND path_suffixes ${suffixes})
    endif()
    set (suffixes "${abi}")
    if (suffixes)
      list (TRANSFORM suffixes PREPEND "lib/python${_PGPS_VERSION}/config-${_PGPS_VERSION}")
    else()
      set (suffixes "lib/python${_PGPS_VERSION}/config-${_PGPS_VERSION}")
    endif()
    list (APPEND path_suffixes ${suffixes})
  elseif (_PGPS_INCLUDE)
    set (suffixes "${abi}")
    if (suffixes)
      list (TRANSFORM suffixes PREPEND "include/python${_PGPS_VERSION}")
    else()
      set (suffixes "include/python${_PGPS_VERSION}")
    endif()
    list (APPEND path_suffixes ${suffixes} include)
  endif()

  set (${_PYTHON_PGPS_PATH_SUFFIXES} ${path_suffixes} PARENT_SCOPE)
endfunction()

function (_PYTHON_GET_NAMES _PYTHON_PGN_NAMES)
  cmake_parse_arguments (PARSE_ARGV 1 _PGN "POSIX;EXECUTABLE;CONFIG;LIBRARY;WIN32;DEBUG" "VERSION" "")

  set (names)

  if (_PGN_WIN32)
    string (REPLACE "." "" _PYTHON_VERSION_NO_DOTS ${_PGN_VERSION})

    set (name python${_PYTHON_VERSION_NO_DOTS})
    if (_PGN_DEBUG)
      string (APPEND name "_d")
    endif()

    list (APPEND names "${name}")
  endif()

  if (_PGN_POSIX)
    if (DEFINED _${_PYTHON_PREFIX}_ABIFLAGS)
      set (abi "${_${_PYTHON_PREFIX}_ABIFLAGS}")
    else()
      if (_PGN_EXECUTABLE OR _PGN_CONFIG)
        set (abi "")
      else()
        set (abi "mu" "m" "u" "")
      endif()
    endif()

    if (abi)
      if (_PGN_CONFIG AND DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        set (abinames "${abi}")
        list (TRANSFORM abinames PREPEND "${CMAKE_LIBRARY_ARCHITECTURE}-python${_PGN_VERSION}")
        list (TRANSFORM abinames APPEND "-config")
        list (APPEND names ${abinames})
      endif()
      set (abinames "${abi}")
      list (TRANSFORM abinames PREPEND "python${_PGN_VERSION}")
      if (_PGN_CONFIG)
        list (TRANSFORM abinames APPEND "-config")
      endif()
      list (APPEND names ${abinames})
    else()
      if (_PGN_CONFIG AND DEFINED CMAKE_LIBRARY_ARCHITECTURE)
        set (abinames "${CMAKE_LIBRARY_ARCHITECTURE}-python${_PGN_VERSION}")
      endif()
      list (APPEND abinames "python${_PGN_VERSION}")
      if (_PGN_CONFIG)
        list (TRANSFORM abinames APPEND "-config")
      endif()
      list (APPEND names ${abinames})
    endif()
  endif()

  set (${_PYTHON_PGN_NAMES} ${names} PARENT_SCOPE)
endfunction()

function (_PYTHON_GET_CONFIG_VAR _PYTHON_PGCV_VALUE NAME)
  unset (${_PYTHON_PGCV_VALUE} PARENT_SCOPE)

  if (NOT NAME MATCHES "^(PREFIX|ABIFLAGS|CONFIGDIR|INCLUDES|LIBS|SOABI)$")
    return()
  endif()

  if (_${_PYTHON_PREFIX}_CONFIG)
    if (NAME STREQUAL "SOABI")
      set (config_flag "--extension-suffix")
    else()
      set (config_flag "--${NAME}")
    endif()
    string (TOLOWER "${config_flag}" config_flag)
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" ${config_flag}
                     RESULT_VARIABLE _result
                     OUTPUT_VARIABLE _values
                     ERROR_QUIET
                     OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (_result)
      unset (_values)
    else()
      if (NAME STREQUAL "INCLUDES")
        # do some clean-up
        string (REGEX MATCHALL "(-I|-iwithsysroot)[ ]*[^ ]+" _values "${_values}")
        string (REGEX REPLACE "(-I|-iwithsysroot)[ ]*" "" _values "${_values}")
        list (REMOVE_DUPLICATES _values)
      elseif (NAME STREQUAL "SOABI")
        # clean-up: remove prefix character and suffix
        string (REGEX REPLACE "^[.-](.+)(${CMAKE_SHARED_LIBRARY_SUFFIX}|\\.(so|pyd))$" "\\1" _values "${_values}")
      endif()
    endif()
  endif()

  if (_${_PYTHON_PREFIX}_EXECUTABLE AND NOT CMAKE_CROSSCOMPILING)
    if (NAME STREQUAL "PREFIX")
      execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; from distutils import sysconfig; sys.stdout.write(';'.join([sysconfig.PREFIX,sysconfig.EXEC_PREFIX,sysconfig.BASE_EXEC_PREFIX]))"
                       RESULT_VARIABLE _result
                       OUTPUT_VARIABLE _values
                       ERROR_QUIET
                       OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (_result)
        unset (_values)
      else()
        list (REMOVE_DUPLICATES _values)
      endif()
    elseif (NAME STREQUAL "INCLUDES")
      execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; from distutils import sysconfig; sys.stdout.write(';'.join([sysconfig.get_python_inc(plat_specific=True),sysconfig.get_python_inc(plat_specific=False)]))"
                       RESULT_VARIABLE _result
                       OUTPUT_VARIABLE _values
                       ERROR_QUIET
                       OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (_result)
        unset (_values)
      endif()
    elseif (NAME STREQUAL "SOABI")
      execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; from distutils import sysconfig;sys.stdout.write(';'.join([sysconfig.get_config_var('SOABI') or '',sysconfig.get_config_var('EXT_SUFFIX') or '']))"
                       RESULT_VARIABLE _result
                       OUTPUT_VARIABLE _soabi
                       ERROR_QUIET
                       OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (_result)
        unset (_values)
      else()
        list (GET _soabi 0 _values)
        if (NOT _values)
          # try to compute SOABI from EXT_SUFFIX
          list (GET _soabi 1 _values)
          if (_values)
            # clean-up: remove prefix character and suffix
            string (REGEX REPLACE "^[.-](.+)(${CMAKE_SHARED_LIBRARY_SUFFIX}|\\.(so|pyd))$" "\\1" _values "${_values}")
          endif()
        endif()
      endif()
    else()
      set (config_flag "${NAME}")
      if (NAME STREQUAL "CONFIGDIR")
        set (config_flag "LIBPL")
      endif()
      execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; from distutils import sysconfig; sys.stdout.write(sysconfig.get_config_var('${config_flag}'))"
                       RESULT_VARIABLE _result
                       OUTPUT_VARIABLE _values
                       ERROR_QUIET
                       OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (_result)
        unset (_values)
      endif()
    endif()
  endif()

  if (config_flag STREQUAL "ABIFLAGS")
    set (${_PYTHON_PGCV_VALUE} "${_values}" PARENT_SCOPE)
    return()
  endif()

  if (NOT _values OR _values STREQUAL "None")
    return()
  endif()

  if (NAME STREQUAL "LIBS")
    # do some clean-up
    string (REGEX MATCHALL "-(l|framework)[ ]*[^ ]+" _values "${_values}")
    # remove elements relative to python library itself
    list (FILTER _values EXCLUDE REGEX "-lpython")
    list (REMOVE_DUPLICATES _values)
  endif()

  set (${_PYTHON_PGCV_VALUE} "${_values}" PARENT_SCOPE)
endfunction()

function (_PYTHON_GET_VERSION)
  cmake_parse_arguments (PARSE_ARGV 0 _PGV "LIBRARY;INCLUDE" "PREFIX" "")

  unset (${_PGV_PREFIX}VERSION PARENT_SCOPE)
  unset (${_PGV_PREFIX}VERSION_MAJOR PARENT_SCOPE)
  unset (${_PGV_PREFIX}VERSION_MINOR PARENT_SCOPE)
  unset (${_PGV_PREFIX}VERSION_PATCH PARENT_SCOPE)
  unset (${_PGV_PREFIX}ABI PARENT_SCOPE)

  if (_PGV_LIBRARY)
    # retrieve version and abi from library name
    if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE)
      # extract version from library name
      if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE MATCHES "python([23])([0-9]+)")
        set (${_PGV_PREFIX}VERSION_MAJOR "${CMAKE_MATCH_1}" PARENT_SCOPE)
        set (${_PGV_PREFIX}VERSION_MINOR "${CMAKE_MATCH_2}" PARENT_SCOPE)
        set (${_PGV_PREFIX}VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}" PARENT_SCOPE)
        set (${_PGV_PREFIX}ABI "" PARENT_SCOPE)
      elseif (_${_PYTHON_PREFIX}_LIBRARY_RELEASE MATCHES "python([23])\\.([0-9]+)([dmu]*)")
        set (${_PGV_PREFIX}VERSION_MAJOR "${CMAKE_MATCH_1}" PARENT_SCOPE)
        set (${_PGV_PREFIX}VERSION_MINOR "${CMAKE_MATCH_2}" PARENT_SCOPE)
        set (${_PGV_PREFIX}VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}" PARENT_SCOPE)
        set (${_PGV_PREFIX}ABI "${CMAKE_MATCH_3}" PARENT_SCOPE)
      endif()
    endif()
  else()
    if (_${_PYTHON_PREFIX}_INCLUDE_DIR)
      # retrieve version from header file
      file (STRINGS "${_${_PYTHON_PREFIX}_INCLUDE_DIR}/patchlevel.h" version
            REGEX "^#define[ \t]+PY_VERSION[ \t]+\"[^\"]+\"")
      string (REGEX REPLACE "^#define[ \t]+PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1"
                            version "${version}")
      string (REGEX MATCHALL "[0-9]+" versions "${version}")
      list (GET versions 0 version_major)
      list (GET versions 1 version_minor)
      list (GET versions 2 version_patch)

      set (${_PGV_PREFIX}VERSION "${version_major}.${version_minor}" PARENT_SCOPE)
      set (${_PGV_PREFIX}VERSION_MAJOR ${version_major} PARENT_SCOPE)
      set (${_PGV_PREFIX}VERSION_MINOR ${version_minor} PARENT_SCOPE)
      set (${_PGV_PREFIX}VERSION_PATCH ${version_patch} PARENT_SCOPE)

      # compute ABI flags
      if (version_major VERSION_GREATER 2)
        file (STRINGS "${_${_PYTHON_PREFIX}_INCLUDE_DIR}/pyconfig.h" config REGEX "(Py_DEBUG|WITH_PYMALLOC|Py_UNICODE_SIZE|MS_WIN32)")
        set (abi)
        if (config MATCHES "#[ ]*define[ ]+MS_WIN32")
          # ABI not used on Windows
          set (abi "")
        else()
          if (config MATCHES "#[ ]*define[ ]+Py_DEBUG[ ]+1")
            string (APPEND abi "d")
          endif()
          if (config MATCHES "#[ ]*define[ ]+WITH_PYMALLOC[ ]+1")
            string (APPEND abi "m")
          endif()
          if (config MATCHES "#[ ]*define[ ]+Py_UNICODE_SIZE[ ]+4")
            string (APPEND abi "u")
          endif()
          set (${_PGV_PREFIX}ABI "${abi}" PARENT_SCOPE)
        endif()
      else()
        # ABI not supported
        set (${_PGV_PREFIX}ABI "" PARENT_SCOPE)
      endif()
    endif()
  endif()
endfunction()


function (_PYTHON_VALIDATE_INTERPRETER)
  if (NOT _${_PYTHON_PREFIX}_EXECUTABLE)
    return()
  endif()

  cmake_parse_arguments (PARSE_ARGV 0 _PVI "EXACT;CHECK_EXISTS" "" "")
  if (_PVI_UNPARSED_ARGUMENTS)
    set (expected_version ${_PVI_UNPARSED_ARGUMENTS})
  else()
    unset (expected_version)
  endif()

  if (_PVI_CHECK_EXISTS AND NOT EXISTS "${_${_PYTHON_PREFIX}_EXECUTABLE}")
    # interpreter does not exist anymore
    set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Cannot find the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
    return()
  endif()

  # validate ABI compatibility
  if (DEFINED _${_PYTHON_PREFIX}_FIND_ABI)
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c
                             "import sys; sys.stdout.write(sys.abiflags)"
                     RESULT_VARIABLE result
                     OUTPUT_VARIABLE abi
                     ERROR_QUIET
                     OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (result)
      # assume ABI is not supported
      set (abi "")
    endif()
    if (NOT abi IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS)
      # incompatible ABI
      set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Wrong ABI for the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
      set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
      return()
    endif()
  endif()

  get_filename_component (python_name "${_${_PYTHON_PREFIX}_EXECUTABLE}" NAME)

  if (expected_version AND NOT python_name STREQUAL "python${expected_version}${abi}${CMAKE_EXECUTABLE_SUFFIX}")
    # executable found must have a specific version
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c
                             "import sys; sys.stdout.write('.'.join([str(x) for x in sys.version_info[:2]]))"
                     RESULT_VARIABLE result
                     OUTPUT_VARIABLE version
                     ERROR_QUIET
                     OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (result)
      # interpreter is not usable
      set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Cannot use the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
      set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
    else()
      if (_PVI_EXACT AND NOT version VERSION_EQUAL expected_version)
        # interpreter has wrong version
        set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Wrong version for the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
        set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
      else()
        # check that version is OK
        string(REGEX REPLACE "^([0-9]+)\\..*$" "\\1" major_version "${version}")
        string(REGEX REPLACE "^([0-9]+)\\.?.*$" "\\1" expected_major_version "${expected_version}")
        if (NOT major_version VERSION_EQUAL expected_major_version
            OR NOT version VERSION_GREATER_EQUAL expected_version)
          set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Wrong version for the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
          set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
        endif()
      endif()
    endif()
    if (NOT _${_PYTHON_PREFIX}_EXECUTABLE)
      return()
    endif()
  else()
    if (NOT python_name STREQUAL "python${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}${CMAKE_EXECUTABLE_SUFFIX}")
      # executable found do not have version in name
      # ensure major version is OK
      execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c
                               "import sys; sys.stdout.write(str(sys.version_info[0]))"
                       RESULT_VARIABLE result
                       OUTPUT_VARIABLE version
                       ERROR_QUIET
                       OUTPUT_STRIP_TRAILING_WHITESPACE)
      if (result OR NOT version EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
        # interpreter not usable or has wrong major version
        if (result)
          set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Cannot use the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
        else()
          set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Wrong major version for the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
        endif()
        set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
        return()
      endif()
    endif()
  endif()

  if (CMAKE_SIZEOF_VOID_P AND "Development" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS
      AND NOT CMAKE_CROSSCOMPILING)
    # In this case, interpreter must have same architecture as environment
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c
                             "import sys, struct; sys.stdout.write(str(struct.calcsize(\"P\")))"
                     RESULT_VARIABLE result
                     OUTPUT_VARIABLE size
                     ERROR_QUIET
                     OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (result OR NOT size EQUAL CMAKE_SIZEOF_VOID_P)
      # interpreter not usable or has wrong architecture
      if (result)
        set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Cannot use the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
      else()
        set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Wrong architecture for the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
      endif()
      set_property (CACHE _${_PYTHON_PREFIX}_EXECUTABLE PROPERTY VALUE "${_PYTHON_PREFIX}_EXECUTABLE-NOTFOUND")
      return()
    endif()
  endif()
endfunction()


function (_PYTHON_VALIDATE_COMPILER expected_version)
  if (NOT _${_PYTHON_PREFIX}_COMPILER)
    return()
  endif()

  cmake_parse_arguments (_PVC "EXACT;CHECK_EXISTS" "" "" ${ARGN})
  if (_PVC_UNPARSED_ARGUMENTS)
    set (major_version FALSE)
    set (expected_version ${_PVC_UNPARSED_ARGUMENTS})
  else()
    set (major_version TRUE)
    set (expected_version ${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR})
    set (_PVC_EXACT TRUE)
  endif()

  if (_PVC_CHECK_EXISTS AND NOT EXISTS "${_${_PYTHON_PREFIX}_COMPILER}")
    # Compiler does not exist anymore
    set (_${_PYTHON_PREFIX}_Compiler_REASON_FAILURE "Cannot find the compiler \"${_${_PYTHON_PREFIX}_COMPILER}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_COMPILER PROPERTY VALUE "${_PYTHON_PREFIX}_COMPILER-NOTFOUND")
    return()
  endif()

  # retrieve python environment version from compiler
  set (working_dir "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PythonCompilerVersion.dir")
  if (major_version)
    # check only major version
    file (WRITE "${working_dir}/version.py" "import sys; sys.stdout.write(str(sys.version_info[0]))")
  else()
    file (WRITE "${working_dir}/version.py" "import sys; sys.stdout.write('.'.join([str(x) for x in sys.version_info[:2]]))\n")
  endif()
  execute_process (COMMAND "${_${_PYTHON_PREFIX}_COMPILER}" /target:exe /embed "${working_dir}/version.py"
                   WORKING_DIRECTORY "${working_dir}"
                   OUTPUT_QUIET
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process (COMMAND "${working_dir}/version"
                   WORKING_DIRECTORY "${working_dir}"
                   RESULT_VARIABLE result
                   OUTPUT_VARIABLE version
                   ERROR_QUIET)
  file (REMOVE_RECURSE "${_${_PYTHON_PREFIX}_VERSION_DIR}")

  if (result OR (_PVC_EXACT AND NOT version VERSION_EQUAL expected_version) OR (version VERSION_LESS expected_version))
    # Compiler not usable or has wrong version
    if (result)
      set (_${_PYTHON_PREFIX}_Compiler_REASON_FAILURE "Cannot use the compiler \"${_${_PYTHON_PREFIX}_COMPILER}\"")
    else()
      set (_${_PYTHON_PREFIX}_Compiler_REASON_FAILURE "Wrong version for the compiler \"${_${_PYTHON_PREFIX}_COMPILER}\"")
    endif()
    set_property (CACHE _${_PYTHON_PREFIX}_COMPILER PROPERTY VALUE "${_PYTHON_PREFIX}_COMPILER-NOTFOUND")
  endif()
endfunction()


function (_PYTHON_VALIDATE_LIBRARY)
  if (NOT _${_PYTHON_PREFIX}_LIBRARY_RELEASE)
    return()
  endif()

  cmake_parse_arguments (PARSE_ARGV 0 _PVL "EXACT;CHECK_EXISTS" "" "")
  if (_PVL_UNPARSED_ARGUMENTS)
    set (expected_version ${_PVL_UNPARSED_ARGUMENTS})
  else()
    unset (expected_version)
  endif()

  if (_PVL_CHECK_EXISTS AND NOT EXISTS "${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}")
    # library does not exist anymore
    set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Cannot find the library \"${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_RELEASE PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_RELEASE-NOTFOUND")
    if (WIN32)
      set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_DEBUG PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_DEBUG-NOTFOUND")
    endif()
    set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
    return()
  endif()

  # retrieve version and abi from library name
  _python_get_version (LIBRARY PREFIX lib_)

  if (DEFINED _${_PYTHON_PREFIX}_FIND_ABI AND NOT lib_ABI IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS)
    # incompatible ABI
    set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Wrong ABI for the library \"${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_RELEASE PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_RELEASE-NOTFOUND")
  else()
    if (expected_version)
      if ((_PVL_EXACT AND NOT lib_VERSION VERSION_EQUAL expected_version) OR (lib_VERSION VERSION_LESS expected_version))
        # library has wrong version
        set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Wrong version for the library \"${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}\"")
        set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_RELEASE PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_RELEASE-NOTFOUND")
      endif()
    else()
      if (NOT lib_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
        # library has wrong major version
        set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Wrong major version for the library \"${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}\"")
        set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_RELEASE PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_RELEASE-NOTFOUND")
      endif()
    endif()
  endif()

  if (NOT _${_PYTHON_PREFIX}_LIBRARY_RELEASE)
    if (WIN32)
      set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_DEBUG PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_DEBUG-NOTFOUND")
    endif()
    unset (_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE CACHE)
    unset (_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG CACHE)
    set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
  endif()
endfunction()


function (_PYTHON_VALIDATE_INCLUDE_DIR)
  if (NOT _${_PYTHON_PREFIX}_INCLUDE_DIR)
    return()
  endif()

  cmake_parse_arguments (PARSE_ARGV 0 _PVID "EXACT;CHECK_EXISTS" "" "")
  if (_PVID_UNPARSED_ARGUMENTS)
    set (expected_version ${_PVID_UNPARSED_ARGUMENTS})
  else()
    unset (expected_version)
  endif()

  if (_PVID_CHECK_EXISTS AND NOT EXISTS "${_${_PYTHON_PREFIX}_INCLUDE_DIR}")
    # include file does not exist anymore
    set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Cannot find the directory \"${_${_PYTHON_PREFIX}_INCLUDE_DIR}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
    return()
  endif()

  # retrieve version from header file
  _python_get_version (INCLUDE PREFIX inc_)

  if (DEFINED _${_PYTHON_PREFIX}_FIND_ABI AND NOT inc_ABI IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS)
    # incompatible ABI
    set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Wrong ABI for the directory \"${_${_PYTHON_PREFIX}_INCLUDE_DIR}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
  else()
    if (expected_version)
      if ((_PVID_EXACT AND NOT inc_VERSION VERSION_EQUAL expected_version) OR (inc_VERSION VERSION_LESS expected_version))
        # include dir has wrong version
        set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Wrong version for the directory \"${_${_PYTHON_PREFIX}_INCLUDE_DIR}\"")
        set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
      endif()
    else()
      if (NOT inc_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
        # include dir has wrong major version
        set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Wrong major version for the directory \"${_${_PYTHON_PREFIX}_INCLUDE_DIR}\"")
        set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
      endif()
    endif()
  endif()
endfunction()


function (_PYTHON_FIND_RUNTIME_LIBRARY _PYTHON_LIB)
  string (REPLACE "_RUNTIME" "" _PYTHON_LIB "${_PYTHON_LIB}")
  # look at runtime part on systems supporting it
  if (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR
      (CMAKE_SYSTEM_NAME MATCHES "MSYS|CYGWIN"
        AND ${_PYTHON_LIB} MATCHES "${CMAKE_IMPORT_LIBRARY_SUFFIX}$"))
    set (CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
    # MSYS has a special syntax for runtime libraries
    if (CMAKE_SYSTEM_NAME MATCHES "MSYS")
      list (APPEND CMAKE_FIND_LIBRARY_PREFIXES "msys-")
    endif()
    find_library (${ARGV})
  endif()
endfunction()


function (_PYTHON_SET_LIBRARY_DIRS _PYTHON_SLD_RESULT)
  unset (_PYTHON_DIRS)
  set (_PYTHON_LIBS ${ARGN})
  foreach (_PYTHON_LIB IN LISTS _PYTHON_LIBS)
    if (${_PYTHON_LIB})
      get_filename_component (_PYTHON_DIR "${${_PYTHON_LIB}}" DIRECTORY)
      list (APPEND _PYTHON_DIRS "${_PYTHON_DIR}")
    endif()
  endforeach()
  if (_PYTHON_DIRS)
    list (REMOVE_DUPLICATES _PYTHON_DIRS)
  endif()
  set (${_PYTHON_SLD_RESULT} ${_PYTHON_DIRS} PARENT_SCOPE)
endfunction()


# If major version is specified, it must be the same as internal major version
if (DEFINED ${_PYTHON_PREFIX}_FIND_VERSION_MAJOR
    AND NOT ${_PYTHON_PREFIX}_FIND_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
  _python_display_failure ("Could NOT find ${_PYTHON_PREFIX}: Wrong major version specified is \"${${_PYTHON_PREFIX}_FIND_VERSION_MAJOR}\", but expected major version is \"${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}\"")
endif()


# handle components
if (NOT ${_PYTHON_PREFIX}_FIND_COMPONENTS)
  set (${_PYTHON_PREFIX}_FIND_COMPONENTS Interpreter)
  set (${_PYTHON_PREFIX}_FIND_REQUIRED_Interpreter TRUE)
endif()
if ("NumPy" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS)
  list (APPEND ${_PYTHON_PREFIX}_FIND_COMPONENTS "Interpreter" "Development")
  list (REMOVE_DUPLICATES ${_PYTHON_PREFIX}_FIND_COMPONENTS)
endif()
foreach (_${_PYTHON_PREFIX}_COMPONENT IN ITEMS Interpreter Compiler Development NumPy)
  set (${_PYTHON_PREFIX}_${_${_PYTHON_PREFIX}_COMPONENT}_FOUND FALSE)
endforeach()
unset (_${_PYTHON_PREFIX}_FIND_VERSIONS)

# Set versions to search
## default: search any version
set (_${_PYTHON_PREFIX}_FIND_VERSIONS ${_${_PYTHON_PREFIX}_VERSIONS})

if (${_PYTHON_PREFIX}_FIND_VERSION_COUNT GREATER 1)
  if (${_PYTHON_PREFIX}_FIND_VERSION_EXACT)
    set (_${_PYTHON_PREFIX}_FIND_VERSIONS ${${_PYTHON_PREFIX}_FIND_VERSION_MAJOR}.${${_PYTHON_PREFIX}_FIND_VERSION_MINOR})
  else()
    unset (_${_PYTHON_PREFIX}_FIND_VERSIONS)
    # add all compatible versions
    foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_VERSIONS)
      if (_${_PYTHON_PREFIX}_VERSION VERSION_GREATER_EQUAL ${_PYTHON_PREFIX}_FIND_VERSION)
        list (APPEND _${_PYTHON_PREFIX}_FIND_VERSIONS ${_${_PYTHON_PREFIX}_VERSION})
      endif()
    endforeach()
  endif()
endif()

# Set ABIs to search
## default: search any ABI
if (_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR VERSION_LESS 3)
  # ABI not supported
  unset (_${_PYTHON_PREFIX}_FIND_ABI)
  set (_${_PYTHON_PREFIX}_ABIFLAGS "")
else()
  unset (_${_PYTHON_PREFIX}_FIND_ABI)
  unset (_${_PYTHON_PREFIX}_ABIFLAGS)
  if (DEFINED ${_PYTHON_PREFIX}_FIND_ABI)
    # normalization
    string (TOUPPER "${${_PYTHON_PREFIX}_FIND_ABI}" _${_PYTHON_PREFIX}_FIND_ABI)
    list (TRANSFORM _${_PYTHON_PREFIX}_FIND_ABI REPLACE "^(TRUE|Y(ES)?|1)$" "ON")
    list (TRANSFORM _${_PYTHON_PREFIX}_FIND_ABI REPLACE "^(FALSE|N(O)?|0)$" "OFF")
    if (NOT _${_PYTHON_PREFIX}_FIND_ABI MATCHES "^(ON|OFF|ANY);(ON|OFF|ANY);(ON|OFF|ANY)$")
      message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: ${${_PYTHON_PREFIX}_FIND_ABI}: invalid value for '${_PYTHON_PREFIX}_FIND_ABI'. Ignore it")
      unset (_${_PYTHON_PREFIX}_FIND_ABI)
    endif()
    _python_get_abiflags (_${_PYTHON_PREFIX}_ABIFLAGS)
  endif()
endif()
unset (${_PYTHON_PREFIX}_SOABI)

# Define lookup strategy
if (_${_PYTHON_PREFIX}_LOOKUP_POLICY STREQUAL "NEW")
  set (_${_PYTHON_PREFIX}_FIND_STRATEGY "LOCATION")
else()
  set (_${_PYTHON_PREFIX}_FIND_STRATEGY "VERSION")
endif()
if (DEFINED ${_PYTHON_PREFIX}_FIND_STRATEGY)
  if (NOT ${_PYTHON_PREFIX}_FIND_STRATEGY MATCHES "^(VERSION|LOCATION)$")
    message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: ${${_PYTHON_PREFIX}_FIND_STRATEGY}: invalid value for '${_PYTHON_PREFIX}_FIND_STRATEGY'. 'VERSION' or 'LOCATION' expected.")
    set (_${_PYTHON_PREFIX}_FIND_STRATEGY "VERSION")
  else()
    set (_${_PYTHON_PREFIX}_FIND_STRATEGY "${${_PYTHON_PREFIX}_FIND_STRATEGY}")
  endif()
endif()

# Python and Anaconda distributions: define which architectures can be used
if (CMAKE_SIZEOF_VOID_P)
  # In this case, search only for 64bit or 32bit
  math (EXPR _${_PYTHON_PREFIX}_ARCH "${CMAKE_SIZEOF_VOID_P} * 8")
  set (_${_PYTHON_PREFIX}_ARCH2 ${_${_PYTHON_PREFIX}_ARCH})
else()
  # architecture unknown, search for both 64bit and 32bit
  set (_${_PYTHON_PREFIX}_ARCH 64)
  set (_${_PYTHON_PREFIX}_ARCH2 32)
endif()

# IronPython support
if (CMAKE_SIZEOF_VOID_P)
  # In this case, search only for 64bit or 32bit
  math (EXPR _${_PYTHON_PREFIX}_ARCH "${CMAKE_SIZEOF_VOID_P} * 8")
  set (_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES ipy${_${_PYTHON_PREFIX}_ARCH} ipy)
else()
  # architecture unknown, search for natural interpreter
  set (_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES ipy)
endif()
set (_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES net45 net40)

# Apple frameworks handling
_python_find_frameworks ()

set (_${_PYTHON_PREFIX}_FIND_FRAMEWORK "FIRST")

if (DEFINED ${_PYTHON_PREFIX}_FIND_FRAMEWORK)
  if (NOT ${_PYTHON_PREFIX}_FIND_FRAMEWORK MATCHES "^(FIRST|LAST|NEVER)$")
    message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: ${${_PYTHON_PREFIX}_FIND_FRAMEWORK}: invalid value for '${_PYTHON_PREFIX}_FIND_FRAMEWORK'. 'FIRST', 'LAST' or 'NEVER' expected. 'FIRST' will be used instead.")
  else()
    set (_${_PYTHON_PREFIX}_FIND_FRAMEWORK ${${_PYTHON_PREFIX}_FIND_FRAMEWORK})
  endif()
elseif (DEFINED CMAKE_FIND_FRAMEWORK)
  if (CMAKE_FIND_FRAMEWORK STREQUAL "ONLY")
    message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: CMAKE_FIND_FRAMEWORK: 'ONLY' value is not supported. 'FIRST' will be used instead.")
  elseif (NOT CMAKE_FIND_FRAMEWORK MATCHES "^(FIRST|LAST|NEVER)$")
    message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: ${CMAKE_FIND_FRAMEWORK}: invalid value for 'CMAKE_FIND_FRAMEWORK'. 'FIRST', 'LAST' or 'NEVER' expected. 'FIRST' will be used instead.")
  else()
    set (_${_PYTHON_PREFIX}_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK})
  endif()
endif()

# Save CMAKE_FIND_APPBUNDLE
if (DEFINED CMAKE_FIND_APPBUNDLE)
  set (_${_PYTHON_PREFIX}_CMAKE_FIND_APPBUNDLE ${CMAKE_FIND_APPBUNDLE})
else()
  unset (_${_PYTHON_PREFIX}_CMAKE_FIND_APPBUNDLE)
endif()
# To avoid app bundle lookup
set (CMAKE_FIND_APPBUNDLE "NEVER")

# Save CMAKE_FIND_FRAMEWORK
if (DEFINED CMAKE_FIND_FRAMEWORK)
  set (_${_PYTHON_PREFIX}_CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK})
else()
  unset (_${_PYTHON_PREFIX}_CMAKE_FIND_FRAMEWORK)
endif()
# To avoid framework lookup
set (CMAKE_FIND_FRAMEWORK "NEVER")

# Windows Registry handling
if (DEFINED ${_PYTHON_PREFIX}_FIND_REGISTRY)
  if (NOT ${_PYTHON_PREFIX}_FIND_REGISTRY MATCHES "^(FIRST|LAST|NEVER)$")
    message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: ${${_PYTHON_PREFIX}_FIND_REGISTRY}: invalid value for '${_PYTHON_PREFIX}_FIND_REGISTRY'. 'FIRST', 'LAST' or 'NEVER' expected. 'FIRST' will be used instead.")
    set (_${_PYTHON_PREFIX}_FIND_REGISTRY "FIRST")
  else()
    set (_${_PYTHON_PREFIX}_FIND_REGISTRY ${${_PYTHON_PREFIX}_FIND_REGISTRY})
  endif()
else()
  set (_${_PYTHON_PREFIX}_FIND_REGISTRY "FIRST")
endif()

# virtual environments recognition
if (DEFINED ENV{VIRTUAL_ENV} OR DEFINED ENV{CONDA_PREFIX})
  if (DEFINED ${_PYTHON_PREFIX}_FIND_VIRTUALENV)
    if (NOT ${_PYTHON_PREFIX}_FIND_VIRTUALENV MATCHES "^(FIRST|ONLY|STANDARD)$")
      message (AUTHOR_WARNING "Find${_PYTHON_PREFIX}: ${${_PYTHON_PREFIX}_FIND_VIRTUALENV}: invalid value for '${_PYTHON_PREFIX}_FIND_VIRTUALENV'. 'FIRST', 'ONLY' or 'STANDARD' expected. 'FIRST' will be used instead.")
      set (_${_PYTHON_PREFIX}_FIND_VIRTUALENV "FIRST")
    else()
      set (_${_PYTHON_PREFIX}_FIND_VIRTUALENV ${${_PYTHON_PREFIX}_FIND_VIRTUALENV})
    endif()
  else()
    set (_${_PYTHON_PREFIX}_FIND_VIRTUALENV FIRST)
  endif()
else()
  set (_${_PYTHON_PREFIX}_FIND_VIRTUALENV STANDARD)
endif()


# Compute search signature
# This signature will be used to check validity of cached variables on new search
set (_${_PYTHON_PREFIX}_SIGNATURE "${${_PYTHON_PREFIX}_ROOT_DIR}:${_${_PYTHON_PREFIX}_FIND_STRATEGY}:${${_PYTHON_PREFIX}_FIND_VIRTUALENV}")
if (NOT WIN32)
  string (APPEND _${_PYTHON_PREFIX}_SIGNATURE ":${${_PYTHON_PREFIX}_USE_STATIC_LIBS}:")
endif()
if (CMAKE_HOST_APPLE)
  string (APPEND _${_PYTHON_PREFIX}_SIGNATURE ":${_${_PYTHON_PREFIX}_FIND_FRAMEWORK}")
endif()
if (CMAKE_HOST_WIN32)
  string (APPEND _${_PYTHON_PREFIX}_SIGNATURE ":${_${_PYTHON_PREFIX}_FIND_REGISTRY}")
endif()


unset (_${_PYTHON_PREFIX}_REQUIRED_VARS)
unset (_${_PYTHON_PREFIX}_CACHED_VARS)
unset (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE)
unset (_${_PYTHON_PREFIX}_Compiler_REASON_FAILURE)
unset (_${_PYTHON_PREFIX}_Development_REASON_FAILURE)
unset (_${_PYTHON_PREFIX}_NumPy_REASON_FAILURE)


# first step, search for the interpreter
if ("Interpreter" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS)
  list (APPEND _${_PYTHON_PREFIX}_CACHED_VARS _${_PYTHON_PREFIX}_EXECUTABLE)
  if (${_PYTHON_PREFIX}_FIND_REQUIRED_Interpreter)
    list (APPEND _${_PYTHON_PREFIX}_REQUIRED_VARS ${_PYTHON_PREFIX}_EXECUTABLE)
  endif()

  if (DEFINED ${_PYTHON_PREFIX}_EXECUTABLE
      AND IS_ABSOLUTE "${${_PYTHON_PREFIX}_EXECUTABLE}")
    if (NOT ${_PYTHON_PREFIX}_EXECUTABLE STREQUAL _${_PYTHON_PREFIX}_EXECUTABLE)
      # invalidate cache properties
      unset (_${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES CACHE)
    endif()
    set (_${_PYTHON_PREFIX}_EXECUTABLE "${${_PYTHON_PREFIX}_EXECUTABLE}" CACHE INTERNAL "")
  elseif (DEFINED _${_PYTHON_PREFIX}_EXECUTABLE)
    # compute interpreter signature and check validity of definition
    string (MD5 __${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE "${_${_PYTHON_PREFIX}_SIGNATURE}:${_${_PYTHON_PREFIX}_EXECUTABLE}")
    if (__${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE STREQUAL _${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE)
      # check version validity
      if (${_PYTHON_PREFIX}_FIND_VERSION_EXACT)
        _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION} EXACT CHECK_EXISTS)
      else()
        _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION} CHECK_EXISTS)
      endif()
    else()
      unset (_${_PYTHON_PREFIX}_EXECUTABLE CACHE)
    endif()
    if (NOT _${_PYTHON_PREFIX}_EXECUTABLE)
      unset (_${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE CACHE)
      unset (_${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES CACHE)
    endif()
  endif()

  if (NOT _${_PYTHON_PREFIX}_EXECUTABLE)
    set (_${_PYTHON_PREFIX}_HINTS "${${_PYTHON_PREFIX}_ROOT_DIR}" ENV ${_PYTHON_PREFIX}_ROOT_DIR)

    if (_${_PYTHON_PREFIX}_FIND_STRATEGY STREQUAL "LOCATION")
      unset (_${_PYTHON_PREFIX}_NAMES)
      unset (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS)
      unset (_${_PYTHON_PREFIX}_REGISTRY_PATHS)

      foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
        # build all executable names
        _python_get_names (_${_PYTHON_PREFIX}_VERSION_NAMES VERSION ${_${_PYTHON_PREFIX}_VERSION} POSIX EXECUTABLE)
        list (APPEND _${_PYTHON_PREFIX}_NAMES ${_${_PYTHON_PREFIX}_VERSION_NAMES})

        # Framework Paths
        _python_get_frameworks (_${_PYTHON_PREFIX}_VERSION_PATHS ${_${_PYTHON_PREFIX}_VERSION})
        list (APPEND _${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_VERSION_PATHS})

        # Registry Paths
        _python_get_registries (_${_PYTHON_PREFIX}_VERSION_PATHS ${_${_PYTHON_PREFIX}_VERSION})
        list (APPEND _${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_VERSION_PATHS}
                     [HKEY_LOCAL_MACHINE\\SOFTWARE\\IronPython\\${_${_PYTHON_PREFIX}_VERSION}\\InstallPath])
      endforeach()
      list (APPEND _${_PYTHON_PREFIX}_NAMES python${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR} python)

      while (TRUE)
        # Virtual environments handling
        if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV MATCHES "^(FIRST|ONLY)$")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ENV VIRTUAL_ENV ENV CONDA_PREFIX
                        PATH_SUFFIXES bin Scripts
                        NO_CMAKE_PATH
                        NO_CMAKE_ENVIRONMENT_PATH
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)

          _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
          if (_${_PYTHON_PREFIX}_EXECUTABLE)
            break()
          endif()
          if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV STREQUAL "ONLY")
            break()
          endif()
        endif()

        # Apple frameworks handling
        if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES bin
                        NO_CMAKE_PATH
                        NO_CMAKE_ENVIRONMENT_PATH
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
          _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
          if (_${_PYTHON_PREFIX}_EXECUTABLE)
            break()
          endif()
        endif()
        # Windows registry
        if (CMAKE_HOST_WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                              ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                        PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
          _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
          if (_${_PYTHON_PREFIX}_EXECUTABLE)
            break()
          endif()
        endif()

        # try using HINTS
        find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                      NAMES ${_${_PYTHON_PREFIX}_NAMES}
                            ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                      NAMES_PER_DIR
                      HINTS ${_${_PYTHON_PREFIX}_HINTS}
                      PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                      NO_SYSTEM_ENVIRONMENT_PATH
                      NO_CMAKE_SYSTEM_PATH)
        _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
        if (_${_PYTHON_PREFIX}_EXECUTABLE)
          break()
        endif()
        # try using standard paths
        find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                      NAMES ${_${_PYTHON_PREFIX}_NAMES}
                            ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                      NAMES_PER_DIR
                      PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES})
        _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
        if (_${_PYTHON_PREFIX}_EXECUTABLE)
          break()
        endif()

        # Apple frameworks handling
        if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                        NAMES_PER_DIR
                        PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES bin
                        NO_DEFAULT_PATH)
          _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
          if (_${_PYTHON_PREFIX}_EXECUTABLE)
            break()
          endif()
        endif()
        # Windows registry
        if (CMAKE_HOST_WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                              ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                        NAMES_PER_DIR
                        PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                        PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_DEFAULT_PATH)
          _python_validate_interpreter (${${_PYTHON_PREFIX}_FIND_VERSION})
          if (_${_PYTHON_PREFIX}_EXECUTABLE)
            break()
          endif()
        endif()

        break()
      endwhile()
    else()
      # look-up for various versions and locations
      foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
        _python_get_names (_${_PYTHON_PREFIX}_NAMES VERSION ${_${_PYTHON_PREFIX}_VERSION} POSIX EXECUTABLE)
        list (APPEND _${_PYTHON_PREFIX}_NAMES python${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}
                                              python)

        _python_get_frameworks (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_VERSION})
        _python_get_registries (_${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_VERSION})

        # Virtual environments handling
        if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV MATCHES "^(FIRST|ONLY)$")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ENV VIRTUAL_ENV ENV CONDA_PREFIX
                        PATH_SUFFIXES bin Scripts
                        NO_CMAKE_PATH
                        NO_CMAKE_ENVIRONMENT_PATH
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
          _python_validate_interpreter (${_${_PYTHON_PREFIX}_VERSION} EXACT)
          if (_${_PYTHON_PREFIX}_EXECUTABLE)
            break()
          endif()
          if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV STREQUAL "ONLY")
            continue()
          endif()
        endif()

        # Apple frameworks handling
        if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES bin
                        NO_CMAKE_PATH
                        NO_CMAKE_ENVIRONMENT_PATH
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
        endif()

        # Windows registry
        if (CMAKE_HOST_WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                              ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                              [HKEY_LOCAL_MACHINE\\SOFTWARE\\IronPython\\${_${_PYTHON_PREFIX}_VERSION}\\InstallPath]
                        PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
        endif()

        _python_validate_interpreter (${_${_PYTHON_PREFIX}_VERSION} EXACT)
        if (_${_PYTHON_PREFIX}_EXECUTABLE)
          break()
        endif()

        # try using HINTS
        find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                      NAMES ${_${_PYTHON_PREFIX}_NAMES}
                            ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                      NAMES_PER_DIR
                      HINTS ${_${_PYTHON_PREFIX}_HINTS}
                      PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                      NO_SYSTEM_ENVIRONMENT_PATH
                      NO_CMAKE_SYSTEM_PATH)
        _python_validate_interpreter (${_${_PYTHON_PREFIX}_VERSION} EXACT)
        if (_${_PYTHON_PREFIX}_EXECUTABLE)
          break()
        endif()
        # try using standard paths.
        # NAMES_PER_DIR is not defined on purpose to have a chance to find
        # expected version.
        # For example, typical systems have 'python' for version 2.* and 'python3'
        # for version 3.*. So looking for names per dir will find, potentially,
        # systematically 'python' (i.e. version 2) even if version 3 is searched.
        if (CMAKE_HOST_WIN32)
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                              python
                              ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES})
        else()
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES})
        endif()
        _python_validate_interpreter (${_${_PYTHON_PREFIX}_VERSION} EXACT)
        if (_${_PYTHON_PREFIX}_EXECUTABLE)
          break()
        endif()

        # Apple frameworks handling
        if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                        NAMES_PER_DIR
                        PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES bin
                        NO_DEFAULT_PATH)
        endif()

        # Windows registry
        if (CMAKE_HOST_WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                        NAMES ${_${_PYTHON_PREFIX}_NAMES}
                              ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES}
                        NAMES_PER_DIR
                        PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                              [HKEY_LOCAL_MACHINE\\SOFTWARE\\IronPython\\${_${_PYTHON_PREFIX}_VERSION}\\InstallPath]
                        PATH_SUFFIXES bin ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_DEFAULT_PATH)
        endif()

        _python_validate_interpreter (${_${_PYTHON_PREFIX}_VERSION} EXACT)
        if (_${_PYTHON_PREFIX}_EXECUTABLE)
          break()
        endif()
      endforeach()

      if (NOT _${_PYTHON_PREFIX}_EXECUTABLE AND
          NOT _${_PYTHON_PREFIX}_FIND_VIRTUALENV STREQUAL "ONLY")
        # No specific version found. Retry with generic names and standard paths.
        # NAMES_PER_DIR is not defined on purpose to have a chance to find
        # expected version.
        # For example, typical systems have 'python' for version 2.* and 'python3'
        # for version 3.*. So looking for names per dir will find, potentially,
        # systematically 'python' (i.e. version 2) even if version 3 is searched.
        find_program (_${_PYTHON_PREFIX}_EXECUTABLE
                      NAMES python${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}
                            python
                            ${_${_PYTHON_PREFIX}_IRON_PYTHON_NAMES})
        _python_validate_interpreter ()
      endif()
    endif()
  endif()

  set (${_PYTHON_PREFIX}_EXECUTABLE "${_${_PYTHON_PREFIX}_EXECUTABLE}")

  # retrieve exact version of executable found
  if (_${_PYTHON_PREFIX}_EXECUTABLE)
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c
                             "import sys; sys.stdout.write('.'.join([str(x) for x in sys.version_info[:3]]))"
                     RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                     OUTPUT_VARIABLE ${_PYTHON_PREFIX}_VERSION
                     ERROR_QUIET
                     OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT _${_PYTHON_PREFIX}_RESULT)
      set (_${_PYTHON_PREFIX}_EXECUTABLE_USABLE TRUE)
    else()
      # Interpreter is not usable
      set (_${_PYTHON_PREFIX}_EXECUTABLE_USABLE FALSE)
      unset (${_PYTHON_PREFIX}_VERSION)
      set (_${_PYTHON_PREFIX}_Interpreter_REASON_FAILURE "Cannot run the interpreter \"${_${_PYTHON_PREFIX}_EXECUTABLE}\"")
    endif()
  endif()

  if (_${_PYTHON_PREFIX}_EXECUTABLE AND _${_PYTHON_PREFIX}_EXECUTABLE_USABLE)
    if (_${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES)
      set (${_PYTHON_PREFIX}_Interpreter_FOUND TRUE)

      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 0 ${_PYTHON_PREFIX}_INTERPRETER_ID)

      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 1 ${_PYTHON_PREFIX}_VERSION_MAJOR)
      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 2 ${_PYTHON_PREFIX}_VERSION_MINOR)
      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 3 ${_PYTHON_PREFIX}_VERSION_PATCH)

      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 4 _${_PYTHON_PREFIX}_ARCH)
      set (_${_PYTHON_PREFIX}_ARCH2 ${_${_PYTHON_PREFIX}_ARCH})

      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 5 _${_PYTHON_PREFIX}_ABIFLAGS)
      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 6 ${_PYTHON_PREFIX}_SOABI)

      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 7 ${_PYTHON_PREFIX}_STDLIB)
      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 8 ${_PYTHON_PREFIX}_STDARCH)
      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 9 ${_PYTHON_PREFIX}_SITELIB)
      list (GET _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES 10 ${_PYTHON_PREFIX}_SITEARCH)
    else()
      string (REGEX MATCHALL "[0-9]+" _${_PYTHON_PREFIX}_VERSIONS "${${_PYTHON_PREFIX}_VERSION}")
      list (GET _${_PYTHON_PREFIX}_VERSIONS 0 ${_PYTHON_PREFIX}_VERSION_MAJOR)
      list (GET _${_PYTHON_PREFIX}_VERSIONS 1 ${_PYTHON_PREFIX}_VERSION_MINOR)
      list (GET _${_PYTHON_PREFIX}_VERSIONS 2 ${_PYTHON_PREFIX}_VERSION_PATCH)

      if (${_PYTHON_PREFIX}_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
        set (${_PYTHON_PREFIX}_Interpreter_FOUND TRUE)

        # Use interpreter version and ABI for future searches to ensure consistency
        set (_${_PYTHON_PREFIX}_FIND_VERSIONS ${${_PYTHON_PREFIX}_VERSION_MAJOR}.${${_PYTHON_PREFIX}_VERSION_MINOR})
        execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; sys.stdout.write(sys.abiflags)"
                         RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                         OUTPUT_VARIABLE _${_PYTHON_PREFIX}_ABIFLAGS
                         ERROR_QUIET
                         OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (_${_PYTHON_PREFIX}_RESULT)
          # assunme ABI is not supported
          set (_${_PYTHON_PREFIX}_ABIFLAGS "")
        endif()
      endif()

      if (${_PYTHON_PREFIX}_Interpreter_FOUND)
        # compute and save interpreter signature
        string (MD5 __${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE "${_${_PYTHON_PREFIX}_SIGNATURE}:${_${_PYTHON_PREFIX}_EXECUTABLE}")
        set (_${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE "${__${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE}" CACHE INTERNAL "")

        if (NOT CMAKE_SIZEOF_VOID_P)
          # determine interpreter architecture
          execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; print(sys.maxsize > 2**32)"
                           RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                           OUTPUT_VARIABLE ${_PYTHON_PREFIX}_IS64BIT
                           ERROR_VARIABLE ${_PYTHON_PREFIX}_IS64BIT)
          if (NOT _${_PYTHON_PREFIX}_RESULT)
            if (${_PYTHON_PREFIX}_IS64BIT)
              set (_${_PYTHON_PREFIX}_ARCH 64)
              set (_${_PYTHON_PREFIX}_ARCH2 64)
            else()
              set (_${_PYTHON_PREFIX}_ARCH 32)
              set (_${_PYTHON_PREFIX}_ARCH2 32)
            endif()
          endif()
        endif()

        # retrieve interpreter identity
        execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -V
                         RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                         OUTPUT_VARIABLE ${_PYTHON_PREFIX}_INTERPRETER_ID
                         ERROR_VARIABLE ${_PYTHON_PREFIX}_INTERPRETER_ID)
        if (NOT _${_PYTHON_PREFIX}_RESULT)
          if (${_PYTHON_PREFIX}_INTERPRETER_ID MATCHES "Anaconda")
            set (${_PYTHON_PREFIX}_INTERPRETER_ID "Anaconda")
          elseif (${_PYTHON_PREFIX}_INTERPRETER_ID MATCHES "Enthought")
            set (${_PYTHON_PREFIX}_INTERPRETER_ID "Canopy")
          else()
            string (REGEX REPLACE "^([^ ]+).*" "\\1" ${_PYTHON_PREFIX}_INTERPRETER_ID "${${_PYTHON_PREFIX}_INTERPRETER_ID}")
            if (${_PYTHON_PREFIX}_INTERPRETER_ID STREQUAL "Python")
              # try to get a more precise ID
              execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; print(sys.copyright)"
                               RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                               OUTPUT_VARIABLE ${_PYTHON_PREFIX}_COPYRIGHT
                               ERROR_QUIET)
              if (${_PYTHON_PREFIX}_COPYRIGHT MATCHES "ActiveState")
                set (${_PYTHON_PREFIX}_INTERPRETER_ID "ActivePython")
              endif()
            endif()
          endif()
        else()
          set (${_PYTHON_PREFIX}_INTERPRETER_ID Python)
        endif()

        # retrieve various package installation directories
        execute_process (COMMAND "${_${_PYTHON_PREFIX}_EXECUTABLE}" -c "import sys; from distutils import sysconfig;sys.stdout.write(';'.join([sysconfig.get_python_lib(plat_specific=False,standard_lib=True),sysconfig.get_python_lib(plat_specific=True,standard_lib=True),sysconfig.get_python_lib(plat_specific=False,standard_lib=False),sysconfig.get_python_lib(plat_specific=True,standard_lib=False)]))"
                        RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                        OUTPUT_VARIABLE _${_PYTHON_PREFIX}_LIBPATHS
                        ERROR_QUIET)
        if (NOT _${_PYTHON_PREFIX}_RESULT)
          list (GET _${_PYTHON_PREFIX}_LIBPATHS 0 ${_PYTHON_PREFIX}_STDLIB)
          list (GET _${_PYTHON_PREFIX}_LIBPATHS 1 ${_PYTHON_PREFIX}_STDARCH)
          list (GET _${_PYTHON_PREFIX}_LIBPATHS 2 ${_PYTHON_PREFIX}_SITELIB)
          list (GET _${_PYTHON_PREFIX}_LIBPATHS 3 ${_PYTHON_PREFIX}_SITEARCH)
        else()
          unset (${_PYTHON_PREFIX}_STDLIB)
          unset (${_PYTHON_PREFIX}_STDARCH)
          unset (${_PYTHON_PREFIX}_SITELIB)
          unset (${_PYTHON_PREFIX}_SITEARCH)
        endif()

        if (_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR VERSION_GREATER_EQUAL 3)
          _python_get_config_var (${_PYTHON_PREFIX}_SOABI SOABI)
        endif()

        # store properties in the cache to speed-up future searches
        set (_${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES
          "${${_PYTHON_PREFIX}_INTERPRETER_ID};${${_PYTHON_PREFIX}_VERSION_MAJOR};${${_PYTHON_PREFIX}_VERSION_MINOR};${${_PYTHON_PREFIX}_VERSION_PATCH};${_${_PYTHON_PREFIX}_ARCH};${_${_PYTHON_PREFIX}_ABIFLAGS};${${_PYTHON_PREFIX}_SOABI};${${_PYTHON_PREFIX}_STDLIB};${${_PYTHON_PREFIX}_STDARCH};${${_PYTHON_PREFIX}_SITELIB};${${_PYTHON_PREFIX}_SITEARCH}" CACHE INTERNAL "${_PYTHON_PREFIX} Properties")
      else()
        unset (_${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE CACHE)
        unset (${_PYTHON_PREFIX}_INTERPRETER_ID)
      endif()
    endif()
  endif()

  _python_mark_as_internal (_${_PYTHON_PREFIX}_EXECUTABLE
                            _${_PYTHON_PREFIX}_INTERPRETER_PROPERTIES
                            _${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE)
endif()


# second step, search for compiler (IronPython)
if ("Compiler" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS)
  list (APPEND _${_PYTHON_PREFIX}_CACHED_VARS _${_PYTHON_PREFIX}_COMPILER)
  if (${_PYTHON_PREFIX}_FIND_REQUIRED_Compiler)
    list (APPEND _${_PYTHON_PREFIX}_REQUIRED_VARS ${_PYTHON_PREFIX}_COMPILER)
  endif()

  if (DEFINED ${_PYTHON_PREFIX}_COMPILER
      AND IS_ABSOLUTE "${${_PYTHON_PREFIX}_COMPILER}")
    set (_${_PYTHON_PREFIX}_COMPILER "${${_PYTHON_PREFIX}_COMPILER}" CACHE INTERNAL "")
  elseif (DEFINED _${_PYTHON_PREFIX}_COMPILER)
    # compute compiler signature and check validity of definition
    string (MD5 __${_PYTHON_PREFIX}_COMPILER_SIGNATURE "${_${_PYTHON_PREFIX}_SIGNATURE}:${_${_PYTHON_PREFIX}_COMPILER}")
    if (__${_PYTHON_PREFIX}_COMPILER_SIGNATURE STREQUAL _${_PYTHON_PREFIX}_COMPILER_SIGNATURE)
      # check version validity
      if (${_PYTHON_PREFIX}_FIND_VERSION_EXACT)
        _python_validate_compiler (${${_PYTHON_PREFIX}_FIND_VERSION} EXACT CHECK_EXISTS)
      else()
        _python_validate_compiler (${${_PYTHON_PREFIX}_FIND_VERSION} CHECK_EXISTS)
      endif()
    else()
      unset (_${_PYTHON_PREFIX}_COMPILER CACHE)
      unset (_${_PYTHON_PREFIX}_COMPILER_SIGNATURE CACHE)
    endif()
  endif()

  if (NOT _${_PYTHON_PREFIX}_COMPILER)
    # IronPython specific artifacts
    # If IronPython interpreter is found, use its path
    unset (_${_PYTHON_PREFIX}_IRON_ROOT)
    if (${_PYTHON_PREFIX}_Interpreter_FOUND AND ${_PYTHON_PREFIX}_INTERPRETER_ID STREQUAL "IronPython")
      get_filename_component (_${_PYTHON_PREFIX}_IRON_ROOT "${${_PYTHON_PREFIX}_EXECUTABLE}" DIRECTORY)
    endif()

    if (_${_PYTHON_PREFIX}_FIND_STRATEGY STREQUAL "LOCATION")
      set (_${_PYTHON_PREFIX}_REGISTRY_PATHS)

      foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
        # Registry Paths
        list (APPEND _${_PYTHON_PREFIX}_REGISTRY_PATHS
                     [HKEY_LOCAL_MACHINE\\SOFTWARE\\IronPython\\${_${_PYTHON_PREFIX}_VERSION}\\InstallPath])
      endforeach()

      while (TRUE)
        if (_${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_COMPILER
                        NAMES ipyc
                        HINTS ${_${_PYTHON_PREFIX}_IRON_ROOT} ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
          _python_validate_compiler (${${_PYTHON_PREFIX}_FIND_VERSION})
          if (_${_PYTHON_PREFIX}_COMPILER)
            break()
          endif()
        endif()

        find_program (_${_PYTHON_PREFIX}_COMPILER
                      NAMES ipyc
                      HINTS ${_${_PYTHON_PREFIX}_IRON_ROOT} ${_${_PYTHON_PREFIX}_HINTS}
                      PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                      NO_SYSTEM_ENVIRONMENT_PATH
                      NO_CMAKE_SYSTEM_PATH)
        _python_validate_compiler (${${_PYTHON_PREFIX}_FIND_VERSION})
        if (_${_PYTHON_PREFIX}_COMPILER)
          break()
        endif()

        if (_${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_COMPILER
                        NAMES ipyc
                        PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_DEFAULT_PATH)
        endif()

        break()
      endwhile()
    else()
      # try using root dir and registry
      foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
        if (_${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_COMPILER
                        NAMES ipyc
                        HINTS ${_${_PYTHON_PREFIX}_IRON_ROOT} ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\IronPython\\${_${_PYTHON_PREFIX}_VERSION}\\InstallPath]
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
          _python_validate_compiler (${_${_PYTHON_PREFIX}_VERSION} EXACT)
          if (_${_PYTHON_PREFIX}_COMPILER)
            break()
          endif()
        endif()

        find_program (_${_PYTHON_PREFIX}_COMPILER
                      NAMES ipyc
                      HINTS ${_${_PYTHON_PREFIX}_IRON_ROOT} ${_${_PYTHON_PREFIX}_HINTS}
                      PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                      NO_SYSTEM_ENVIRONMENT_PATH
                      NO_CMAKE_SYSTEM_PATH)
        _python_validate_compiler (${_${_PYTHON_PREFIX}_VERSION} EXACT)
        if (_${_PYTHON_PREFIX}_COMPILER)
          break()
        endif()

        if (_${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_COMPILER
                        NAMES ipyc
                        PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\IronPython\\${_${_PYTHON_PREFIX}_VERSION}\\InstallPath]
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES}
                        NO_DEFAULT_PATH)
          _python_validate_compiler (${_${_PYTHON_PREFIX}_VERSION} EXACT)
          if (_${_PYTHON_PREFIX}_COMPILER)
            break()
          endif()
        endif()
      endforeach()

      # no specific version found, re-try in standard paths
      find_program (_${_PYTHON_PREFIX}_COMPILER
                    NAMES ipyc
                    HINTS ${_${_PYTHON_PREFIX}_IRON_ROOT} ${_${_PYTHON_PREFIX}_HINTS}
                    PATH_SUFFIXES ${_${_PYTHON_PREFIX}_IRON_PYTHON_PATH_SUFFIXES})
    endif()
  endif()

  set (${_PYTHON_PREFIX}_COMPILER "${_${_PYTHON_PREFIX}_COMPILER}")

  if (_${_PYTHON_PREFIX}_COMPILER)
    # retrieve python environment version from compiler
    set (_${_PYTHON_PREFIX}_VERSION_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PythonCompilerVersion.dir")
    file (WRITE "${_${_PYTHON_PREFIX}_VERSION_DIR}/version.py" "import sys; sys.stdout.write('.'.join([str(x) for x in sys.version_info[:3]]))\n")
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_COMPILER}" /target:exe /embed "${_${_PYTHON_PREFIX}_VERSION_DIR}/version.py"
                     WORKING_DIRECTORY "${_${_PYTHON_PREFIX}_VERSION_DIR}"
                     OUTPUT_QUIET
                     ERROR_QUIET)
    execute_process (COMMAND "${_${_PYTHON_PREFIX}_VERSION_DIR}/version"
                     WORKING_DIRECTORY "${_${_PYTHON_PREFIX}_VERSION_DIR}"
                     RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                     OUTPUT_VARIABLE _${_PYTHON_PREFIX}_VERSION
                     ERROR_QUIET)
    if (NOT _${_PYTHON_PREFIX}_RESULT)
      set (_${_PYTHON_PREFIX}_COMPILER_USABLE TRUE)
      string (REGEX MATCHALL "[0-9]+" _${_PYTHON_PREFIX}_VERSIONS "${_${_PYTHON_PREFIX}_VERSION}")
      list (GET _${_PYTHON_PREFIX}_VERSIONS 0 _${_PYTHON_PREFIX}_VERSION_MAJOR)
      list (GET _${_PYTHON_PREFIX}_VERSIONS 1 _${_PYTHON_PREFIX}_VERSION_MINOR)
      list (GET _${_PYTHON_PREFIX}_VERSIONS 2 _${_PYTHON_PREFIX}_VERSION_PATCH)

      if (NOT ${_PYTHON_PREFIX}_Interpreter_FOUND)
        # set public version information
        set (${_PYTHON_PREFIX}_VERSION ${_${_PYTHON_PREFIX}_VERSION})
        set (${_PYTHON_PREFIX}_VERSION_MAJOR ${_${_PYTHON_PREFIX}_VERSION_MAJOR})
        set (${_PYTHON_PREFIX}_VERSION_MINOR ${_${_PYTHON_PREFIX}_VERSION_MINOR})
        set (${_PYTHON_PREFIX}_VERSION_PATCH ${_${_PYTHON_PREFIX}_VERSION_PATCH})
      endif()
    else()
      # compiler not usable
      set (_${_PYTHON_PREFIX}_COMPILER_USABLE FALSE)
      set (_${_PYTHON_PREFIX}_Compiler_REASON_FAILURE "Cannot run the compiler \"${_${_PYTHON_PREFIX}_COMPILER}\"")
    endif()
    file (REMOVE_RECURSE "${_${_PYTHON_PREFIX}_VERSION_DIR}")
  endif()

  if (_${_PYTHON_PREFIX}_COMPILER AND _${_PYTHON_PREFIX}_COMPILER_USABLE)
    if (${_PYTHON_PREFIX}_Interpreter_FOUND)
      # Compiler must be compatible with interpreter
      if (${_${_PYTHON_PREFIX}_VERSION_MAJOR}.${_${_PYTHON_PREFIX}_VERSION_MINOR} VERSION_EQUAL ${${_PYTHON_PREFIX}_VERSION_MAJOR}.${${_PYTHON_PREFIX}_VERSION_MINOR})
        set (${_PYTHON_PREFIX}_Compiler_FOUND TRUE)
      endif()
    elseif (${_PYTHON_PREFIX}_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
      set (${_PYTHON_PREFIX}_Compiler_FOUND TRUE)
    # Use compiler version for future searches to ensure consistency
    set (_${_PYTHON_PREFIX}_FIND_VERSIONS ${${_PYTHON_PREFIX}_VERSION_MAJOR}.${${_PYTHON_PREFIX}_VERSION_MINOR})
    endif()
  endif()

  if (${_PYTHON_PREFIX}_Compiler_FOUND)
    # compute and save compiler signature
    string (MD5 __${_PYTHON_PREFIX}_COMPILER_SIGNATURE "${_${_PYTHON_PREFIX}_SIGNATURE}:${_${_PYTHON_PREFIX}_COMPILER}")
    set (_${_PYTHON_PREFIX}_COMPILER_SIGNATURE "${__${_PYTHON_PREFIX}_COMPILER_SIGNATURE}" CACHE INTERNAL "")

    set (${_PYTHON_PREFIX}_COMPILER_ID IronPython)
  else()
    unset (_${_PYTHON_PREFIX}_COMPILER_SIGNATURE CACHE)
    unset (${_PYTHON_PREFIX}_COMPILER_ID)
  endif()

  _python_mark_as_internal (_${_PYTHON_PREFIX}_COMPILER
                            _${_PYTHON_PREFIX}_COMPILER_SIGNATURE)
endif()


# third step, search for the development artifacts
## Development environment is not compatible with IronPython interpreter
if ("Development" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS
    AND NOT ${_PYTHON_PREFIX}_INTERPRETER_ID STREQUAL "IronPython")
  list (APPEND _${_PYTHON_PREFIX}_CACHED_VARS _${_PYTHON_PREFIX}_LIBRARY_RELEASE
                                              _${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE
                                              _${_PYTHON_PREFIX}_LIBRARY_DEBUG
                                              _${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG
                                              _${_PYTHON_PREFIX}_INCLUDE_DIR)
  if (${_PYTHON_PREFIX}_FIND_REQUIRED_Development)
    list (APPEND _${_PYTHON_PREFIX}_REQUIRED_VARS ${_PYTHON_PREFIX}_LIBRARIES
                                                  ${_PYTHON_PREFIX}_INCLUDE_DIRS)
  endif()

  if (DEFINED _${_PYTHON_PREFIX}_LIBRARY_RELEASE OR DEFINED _${_PYTHON_PREFIX}_INCLUDE_DIR)
    # compute development signature and check validity of definition
    string (MD5 __${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE "${_${_PYTHON_PREFIX}_SIGNATURE}:${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}:${_${_PYTHON_PREFIX}_INCLUDE_DIR}")
    if (WIN32 AND NOT DEFINED _${_PYTHON_PREFIX}_LIBRARY_DEBUG)
      set (_${_PYTHON_PREFIX}_LIBRARY_DEBUG "${_PYTHON_PREFIX}_LIBRARY_DEBUG-NOTFOUND" CACHE INTERNAL "")
    endif()
    if (NOT DEFINED _${_PYTHON_PREFIX}_INCLUDE_DIR)
      set (_${_PYTHON_PREFIX}_INCLUDE_DIR "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND" CACHE INTERNAL "")
    endif()
    if (__${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE STREQUAL _${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE)
      # check version validity
      if (${_PYTHON_PREFIX}_FIND_VERSION_EXACT)
        _python_validate_library (${${_PYTHON_PREFIX}_FIND_VERSION} EXACT CHECK_EXISTS)
        _python_validate_include_dir (${${_PYTHON_PREFIX}_FIND_VERSION} EXACT CHECK_EXISTS)
      else()
        _python_validate_library (${${_PYTHON_PREFIX}_FIND_VERSION} CHECK_EXISTS)
        _python_validate_include_dir (${${_PYTHON_PREFIX}_FIND_VERSION} CHECK_EXISTS)
      endif()
    else()
      unset (_${_PYTHON_PREFIX}_LIBRARY_RELEASE CACHE)
      unset (_${_PYTHON_PREFIX}_LIBRARY_DEBUG CACHE)
      unset (_${_PYTHON_PREFIX}_INCLUDE_DIR CACHE)
    endif()
  endif()
  if (NOT _${_PYTHON_PREFIX}_LIBRARY_RELEASE OR NOT _${_PYTHON_PREFIX}_INCLUDE_DIR)
    unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
    unset (_${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE CACHE)
  endif()

  if (DEFINED ${_PYTHON_PREFIX}_LIBRARY
      AND IS_ABSOLUTE "${${_PYTHON_PREFIX}_LIBRARY}")
    set (_${_PYTHON_PREFIX}_LIBRARY_RELEASE "${${_PYTHON_PREFIX}_LIBRARY}" CACHE INTERNAL "")
    unset (_${_PYTHON_PREFIX}_LIBRARY_DEBUG CACHE)
    unset (_${_PYTHON_PREFIX}_INCLUDE_DIR CACHE)
  endif()
  if (DEFINED ${_PYTHON_PREFIX}_INCLUDE_DIR
      AND IS_ABSOLUTE "${${_PYTHON_PREFIX}_INCLUDE_DIR}")
    set (_${_PYTHON_PREFIX}_INCLUDE_DIR "${${_PYTHON_PREFIX}_INCLUDE_DIR}" CACHE INTERNAL "")
  endif()

  # Support preference of static libs by adjusting CMAKE_FIND_LIBRARY_SUFFIXES
  unset (_${_PYTHON_PREFIX}_CMAKE_FIND_LIBRARY_SUFFIXES)
  if (DEFINED ${_PYTHON_PREFIX}_USE_STATIC_LIBS AND NOT WIN32)
    set(_${_PYTHON_PREFIX}_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
    if(${_PYTHON_PREFIX}_USE_STATIC_LIBS)
      set (CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
      list (REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
  endif()

  if (NOT _${_PYTHON_PREFIX}_LIBRARY_RELEASE OR NOT _${_PYTHON_PREFIX}_INCLUDE_DIR)
    # if python interpreter is found, use it to look-up for artifacts
    # to ensure consistency between interpreter and development environments.
    # If not, try to locate a compatible config tool
    if (NOT ${_PYTHON_PREFIX}_Interpreter_FOUND OR CMAKE_CROSSCOMPILING)
      set (_${_PYTHON_PREFIX}_HINTS "${${_PYTHON_PREFIX}_ROOT_DIR}" ENV ${_PYTHON_PREFIX}_ROOT_DIR)
      unset (_${_PYTHON_PREFIX}_VIRTUALENV_PATHS)
      if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV MATCHES "^(FIRST|ONLY)$")
        set (_${_PYTHON_PREFIX}_VIRTUALENV_PATHS ENV VIRTUAL_ENV ENV CONDA_PREFIX)
      endif()
      unset (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS)

      if (_${_PYTHON_PREFIX}_FIND_STRATEGY STREQUAL "LOCATION")
        set (_${_PYTHON_PREFIX}_CONFIG_NAMES)

        foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
          _python_get_names (_${_PYTHON_PREFIX}_VERSION_NAMES VERSION ${_${_PYTHON_PREFIX}_VERSION} POSIX CONFIG)
          list (APPEND _${_PYTHON_PREFIX}_CONFIG_NAMES ${_${_PYTHON_PREFIX}_VERSION_NAMES})

          # Framework Paths
          _python_get_frameworks (_${_PYTHON_PREFIX}_VERSION_PATHS ${_${_PYTHON_PREFIX}_VERSION})
          list (APPEND _${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_VERSION_PATHS})
        endforeach()

        # Apple frameworks handling
        if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
          find_program (_${_PYTHON_PREFIX}_CONFIG
                        NAMES ${_${_PYTHON_PREFIX}_CONFIG_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                              ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES bin
                        NO_CMAKE_PATH
                        NO_CMAKE_ENVIRONMENT_PATH
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
        endif()

        find_program (_${_PYTHON_PREFIX}_CONFIG
                      NAMES ${_${_PYTHON_PREFIX}_CONFIG_NAMES}
                      NAMES_PER_DIR
                      HINTS ${_${_PYTHON_PREFIX}_HINTS}
                      PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                      PATH_SUFFIXES bin)

        # Apple frameworks handling
        if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
          find_program (_${_PYTHON_PREFIX}_CONFIG
                        NAMES ${_${_PYTHON_PREFIX}_CONFIG_NAMES}
                        NAMES_PER_DIR
                        PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES bin
                        NO_DEFAULT_PATH)
        endif()

        if (_${_PYTHON_PREFIX}_CONFIG)
          execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" --help
                           RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                           OUTPUT_VARIABLE __${_PYTHON_PREFIX}_HELP
                           ERROR_QUIET
                           OUTPUT_STRIP_TRAILING_WHITESPACE)
          if (_${_PYTHON_PREFIX}_RESULT)
            # assume config tool is not usable
            unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
          endif()
        endif()

        if (_${_PYTHON_PREFIX}_CONFIG)
          execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" --abiflags
                           RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                           OUTPUT_VARIABLE __${_PYTHON_PREFIX}_ABIFLAGS
                           ERROR_QUIET
                           OUTPUT_STRIP_TRAILING_WHITESPACE)
          if (_${_PYTHON_PREFIX}_RESULT)
            # assume ABI is not supported
            set (__${_PYTHON_PREFIX}_ABIFLAGS "")
          endif()
          if (DEFINED _${_PYTHON_PREFIX}_FIND_ABI AND NOT __${_PYTHON_PREFIX}_ABIFLAGS IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS)
            # Wrong ABI
            unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
          endif()
        endif()

        if (_${_PYTHON_PREFIX}_CONFIG AND DEFINED CMAKE_LIBRARY_ARCHITECTURE)
          # check that config tool match library architecture
          execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" --configdir
                           RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                           OUTPUT_VARIABLE _${_PYTHON_PREFIX}_CONFIGDIR
                           ERROR_QUIET
                           OUTPUT_STRIP_TRAILING_WHITESPACE)
          if (_${_PYTHON_PREFIX}_RESULT)
            unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
          else()
            string(FIND "${_${_PYTHON_PREFIX}_CONFIGDIR}" "${CMAKE_LIBRARY_ARCHITECTURE}" _${_PYTHON_PREFIX}_RESULT)
            if (_${_PYTHON_PREFIX}_RESULT EQUAL -1)
              unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
            endif()
          endif()
        endif()
      else()
        foreach (_${_PYTHON_PREFIX}_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
          # try to use pythonX.Y-config tool
          _python_get_names (_${_PYTHON_PREFIX}_CONFIG_NAMES VERSION ${_${_PYTHON_PREFIX}_VERSION} POSIX CONFIG)

          # Framework Paths
          _python_get_frameworks (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_VERSION})

          # Apple frameworks handling
          if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
            find_program (_${_PYTHON_PREFIX}_CONFIG
                          NAMES ${_${_PYTHON_PREFIX}_CONFIG_NAMES}
                          NAMES_PER_DIR
                          HINTS ${_${_PYTHON_PREFIX}_HINTS}
                          PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                                ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                          PATH_SUFFIXES bin
                          NO_CMAKE_PATH
                          NO_CMAKE_ENVIRONMENT_PATH
                          NO_SYSTEM_ENVIRONMENT_PATH
                          NO_CMAKE_SYSTEM_PATH)
          endif()

          find_program (_${_PYTHON_PREFIX}_CONFIG
                        NAMES ${_${_PYTHON_PREFIX}_CONFIG_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                        PATH_SUFFIXES bin)

          # Apple frameworks handling
          if (CMAKE_HOST_APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
            find_program (_${_PYTHON_PREFIX}_CONFIG
                          NAMES ${_${_PYTHON_PREFIX}_CONFIG_NAMES}
                          NAMES_PER_DIR
                          PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                          PATH_SUFFIXES bin
                          NO_DEFAULT_PATH)
          endif()

          unset (_${_PYTHON_PREFIX}_CONFIG_NAMES)

          if (_${_PYTHON_PREFIX}_CONFIG)
            execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" --help
                             RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                             OUTPUT_VARIABLE __${_PYTHON_PREFIX}_HELP
                             ERROR_QUIET
                             OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (_${_PYTHON_PREFIX}_RESULT)
              # assume config tool is not usable
              unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
            endif()
          endif()

          if (NOT _${_PYTHON_PREFIX}_CONFIG)
            continue()
          endif()

          execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" --abiflags
                           RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                           OUTPUT_VARIABLE __${_PYTHON_PREFIX}_ABIFLAGS
                           ERROR_QUIET
                           OUTPUT_STRIP_TRAILING_WHITESPACE)
          if (_${_PYTHON_PREFIX}_RESULT)
            # assume ABI is not supported
            set (__${_PYTHON_PREFIX}_ABIFLAGS "")
          endif()
          if (DEFINED _${_PYTHON_PREFIX}_FIND_ABI AND NOT __${_PYTHON_PREFIX}_ABIFLAGS IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS)
            # Wrong ABI
            unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
            continue()
          endif()

          if (_${_PYTHON_PREFIX}_CONFIG AND DEFINED CMAKE_LIBRARY_ARCHITECTURE)
            # check that config tool match library architecture
            execute_process (COMMAND "${_${_PYTHON_PREFIX}_CONFIG}" --configdir
                             RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
                             OUTPUT_VARIABLE _${_PYTHON_PREFIX}_CONFIGDIR
                             ERROR_QUIET
                             OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (_${_PYTHON_PREFIX}_RESULT)
              unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
              continue()
            endif()
            string (FIND "${_${_PYTHON_PREFIX}_CONFIGDIR}" "${CMAKE_LIBRARY_ARCHITECTURE}" _${_PYTHON_PREFIX}_RESULT)
            if (_${_PYTHON_PREFIX}_RESULT EQUAL -1)
              unset (_${_PYTHON_PREFIX}_CONFIG CACHE)
              continue()
            endif()
          endif()

          if (_${_PYTHON_PREFIX}_CONFIG)
            break()
          endif()
        endforeach()
      endif()
    endif()
  endif()

  if (NOT _${_PYTHON_PREFIX}_LIBRARY_RELEASE)
    if ((${_PYTHON_PREFIX}_Interpreter_FOUND AND NOT CMAKE_CROSSCOMPILING) OR _${_PYTHON_PREFIX}_CONFIG)
      # retrieve root install directory
      _python_get_config_var (_${_PYTHON_PREFIX}_PREFIX PREFIX)

      # enforce current ABI
      _python_get_config_var (_${_PYTHON_PREFIX}_ABIFLAGS ABIFLAGS)

      set (_${_PYTHON_PREFIX}_HINTS "${_${_PYTHON_PREFIX}_PREFIX}")

      # retrieve library
      ## compute some paths and artifact names
      if (_${_PYTHON_PREFIX}_CONFIG)
        string (REGEX REPLACE "^.+python([0-9.]+)[a-z]*-config" "\\1" _${_PYTHON_PREFIX}_VERSION "${_${_PYTHON_PREFIX}_CONFIG}")
      else()
        set (_${_PYTHON_PREFIX}_VERSION "${${_PYTHON_PREFIX}_VERSION_MAJOR}.${${_PYTHON_PREFIX}_VERSION_MINOR}")
      endif()
      _python_get_path_suffixes (_${_PYTHON_PREFIX}_PATH_SUFFIXES VERSION ${_${_PYTHON_PREFIX}_VERSION} LIBRARY)
      _python_get_names (_${_PYTHON_PREFIX}_LIB_NAMES VERSION ${_${_PYTHON_PREFIX}_VERSION} WIN32 POSIX LIBRARY)

      _python_get_config_var (_${_PYTHON_PREFIX}_CONFIGDIR CONFIGDIR)
      list (APPEND _${_PYTHON_PREFIX}_HINTS "${_${_PYTHON_PREFIX}_CONFIGDIR}")

      list (APPEND _${_PYTHON_PREFIX}_HINTS "${${_PYTHON_PREFIX}_ROOT_DIR}" ENV ${_PYTHON_PREFIX}_ROOT_DIR)

      find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                    NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                    NAMES_PER_DIR
                    HINTS ${_${_PYTHON_PREFIX}_HINTS}
                    PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                    NO_SYSTEM_ENVIRONMENT_PATH
                    NO_CMAKE_SYSTEM_PATH)
    endif()

    # Rely on HINTS and standard paths if interpreter or config tool failed to locate artifacts
    if (NOT _${_PYTHON_PREFIX}_LIBRARY_RELEASE)
      set (_${_PYTHON_PREFIX}_HINTS "${${_PYTHON_PREFIX}_ROOT_DIR}" ENV ${_PYTHON_PREFIX}_ROOT_DIR)

      unset (_${_PYTHON_PREFIX}_VIRTUALENV_PATHS)
      if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV MATCHES "^(FIRST|ONLY)$")
        set (_${_PYTHON_PREFIX}_VIRTUALENV_PATHS ENV VIRTUAL_ENV ENV CONDA_PREFIX)
      endif()

      if (_${_PYTHON_PREFIX}_FIND_STRATEGY STREQUAL "LOCATION")
        unset (_${_PYTHON_PREFIX}_LIB_NAMES)
        unset (_${_PYTHON_PREFIX}_LIB_NAMES_DEBUG)
        unset (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS)
        unset (_${_PYTHON_PREFIX}_REGISTRY_PATHS)
        unset (_${_PYTHON_PREFIX}_PATH_SUFFIXES)

        foreach (_${_PYTHON_PREFIX}_LIB_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
          # library names
          _python_get_names (_${_PYTHON_PREFIX}_VERSION_NAMES VERSION ${_${_PYTHON_PREFIX}_LIB_VERSION} WIN32 POSIX LIBRARY)
          list (APPEND _${_PYTHON_PREFIX}_LIB_NAMES ${_${_PYTHON_PREFIX}_VERSION_NAMES})
          _python_get_names (_${_PYTHON_PREFIX}_VERSION_NAMES VERSION ${_${_PYTHON_PREFIX}_LIB_VERSION} WIN32 DEBUG)
          list (APPEND _${_PYTHON_PREFIX}_LIB_NAMES_DEBUG ${_${_PYTHON_PREFIX}_VERSION_NAMES})

          # Framework Paths
          _python_get_frameworks (_${_PYTHON_PREFIX}_VERSION_PATHS ${_${_PYTHON_PREFIX}_LIB_VERSION})
          list (APPEND _${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_VERSION_PATHS})

          # Registry Paths
          _python_get_registries (_${_PYTHON_PREFIX}_VERSION_PATHS ${_${_PYTHON_PREFIX}_LIB_VERSION})
          list (APPEND _${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_VERSION_PATHS})

          # Paths suffixes
          _python_get_path_suffixes (_${_PYTHON_PREFIX}_VERSION_PATHS VERSION ${_${_PYTHON_PREFIX}_LIB_VERSION} LIBRARY)
          list (APPEND _${_PYTHON_PREFIX}_PATH_SUFFIXES ${_${_PYTHON_PREFIX}_VERSION_PATHS})
        endforeach()

        if (APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
          find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                        NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                              ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                        NO_CMAKE_PATH
                        NO_CMAKE_ENVIRONMENT_PATH
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
        endif()

        if (WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
          find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                        NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                              ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)
        endif()

        # search in HINTS locations
        find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                      NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                      NAMES_PER_DIR
                      HINTS ${_${_PYTHON_PREFIX}_HINTS}
                      PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                      PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                      NO_SYSTEM_ENVIRONMENT_PATH
                      NO_CMAKE_SYSTEM_PATH)

        if (APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
          set (__${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS})
        else()
          unset (__${_PYTHON_PREFIX}_FRAMEWORK_PATHS)
        endif()

        if (WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
          set (__${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS})
        else()
          unset (__${_PYTHON_PREFIX}_REGISTRY_PATHS)
        endif()

        # search in all default paths
        find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                      NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                      NAMES_PER_DIR
                      PATHS ${__${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                            ${__${_PYTHON_PREFIX}_REGISTRY_PATHS}
                      PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES})
      else()
        foreach (_${_PYTHON_PREFIX}_LIB_VERSION IN LISTS _${_PYTHON_PREFIX}_FIND_VERSIONS)
          _python_get_names (_${_PYTHON_PREFIX}_LIB_NAMES VERSION ${_${_PYTHON_PREFIX}_LIB_VERSION} WIN32 POSIX LIBRARY)
          _python_get_names (_${_PYTHON_PREFIX}_LIB_NAMES_DEBUG VERSION ${_${_PYTHON_PREFIX}_LIB_VERSION} WIN32 DEBUG)

          _python_get_frameworks (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_LIB_VERSION})
          _python_get_registries (_${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_LIB_VERSION})

          _python_get_path_suffixes (_${_PYTHON_PREFIX}_PATH_SUFFIXES VERSION ${_${_PYTHON_PREFIX}_LIB_VERSION} LIBRARY)

          if (APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
            find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                          NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                          NAMES_PER_DIR
                          HINTS ${_${_PYTHON_PREFIX}_HINTS}
                          PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                                ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                          PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                          NO_CMAKE_PATH
                          NO_CMAKE_ENVIRONMENT_PATH
                          NO_SYSTEM_ENVIRONMENT_PATH
                          NO_CMAKE_SYSTEM_PATH)
          endif()

          if (WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
            find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                          NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                          NAMES_PER_DIR
                          HINTS ${_${_PYTHON_PREFIX}_HINTS}
                          PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                                ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                          PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                          NO_SYSTEM_ENVIRONMENT_PATH
                          NO_CMAKE_SYSTEM_PATH)
          endif()

          # search in HINTS locations
          find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                        NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                        NAMES_PER_DIR
                        HINTS ${_${_PYTHON_PREFIX}_HINTS}
                        PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                        NO_SYSTEM_ENVIRONMENT_PATH
                        NO_CMAKE_SYSTEM_PATH)

         if (APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
           set (__${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS})
         else()
           unset (__${_PYTHON_PREFIX}_FRAMEWORK_PATHS)
         endif()

         if (WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
           set (__${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS})
         else()
           unset (__${_PYTHON_PREFIX}_REGISTRY_PATHS)
         endif()

         # search in all default paths
         find_library (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                        NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                        NAMES_PER_DIR
                        PATHS ${__${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                              ${__${_PYTHON_PREFIX}_REGISTRY_PATHS}
                        PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES})

          if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE)
            break()
          endif()
        endforeach()
      endif()
    endif()
  endif()

  # finalize library version information
  _python_get_version (LIBRARY PREFIX _${_PYTHON_PREFIX}_)

  set (${_PYTHON_PREFIX}_LIBRARY_RELEASE "${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}")

  if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE AND NOT EXISTS "${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}")
    set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Cannot find the library \"${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_LIBRARY_RELEASE PROPERTY VALUE "${_PYTHON_PREFIX}_LIBRARY_RELEASE-NOTFOUND")
  endif()

  set (_${_PYTHON_PREFIX}_HINTS "${${_PYTHON_PREFIX}_ROOT_DIR}" ENV ${_PYTHON_PREFIX}_ROOT_DIR)

  if (WIN32 AND _${_PYTHON_PREFIX}_LIBRARY_RELEASE)
    # search for debug library
    # use release library location as a hint
    _python_get_names (_${_PYTHON_PREFIX}_LIB_NAMES_DEBUG VERSION ${_${_PYTHON_PREFIX}_VERSION} WIN32 DEBUG)
    get_filename_component (_${_PYTHON_PREFIX}_PATH "${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}" DIRECTORY)
    find_library (_${_PYTHON_PREFIX}_LIBRARY_DEBUG
                  NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES_DEBUG}
                  NAMES_PER_DIR
                  HINTS "${_${_PYTHON_PREFIX}_PATH}" ${_${_PYTHON_PREFIX}_HINTS}
                  NO_DEFAULT_PATH)
  endif()

  # retrieve runtime libraries
  if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE)
    _python_get_names (_${_PYTHON_PREFIX}_LIB_NAMES VERSION ${_${_PYTHON_PREFIX}_VERSION} WIN32 POSIX LIBRARY)
    get_filename_component (_${_PYTHON_PREFIX}_PATH "${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}" DIRECTORY)
    get_filename_component (_${_PYTHON_PREFIX}_PATH2 "${_${_PYTHON_PREFIX}_PATH}" DIRECTORY)
    _python_find_runtime_library (_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE
                                  NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES}
                                  NAMES_PER_DIR
                                  HINTS "${_${_PYTHON_PREFIX}_PATH}" "${_${_PYTHON_PREFIX}_PATH2}" ${_${_PYTHON_PREFIX}_HINTS}
                                  PATH_SUFFIXES bin)
  endif()
  if (_${_PYTHON_PREFIX}_LIBRARY_DEBUG)
    _python_get_names (_${_PYTHON_PREFIX}_LIB_NAMES_DEBUG VERSION ${_${_PYTHON_PREFIX}_VERSION} WIN32 DEBUG)
    get_filename_component (_${_PYTHON_PREFIX}_PATH "${_${_PYTHON_PREFIX}_LIBRARY_DEBUG}" DIRECTORY)
    get_filename_component (_${_PYTHON_PREFIX}_PATH2 "${_${_PYTHON_PREFIX}_PATH}" DIRECTORY)
    _python_find_runtime_library (_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG
                                  NAMES ${_${_PYTHON_PREFIX}_LIB_NAMES_DEBUG}
                                  NAMES_PER_DIR
                                  HINTS "${_${_PYTHON_PREFIX}_PATH}" "${_${_PYTHON_PREFIX}_PATH2}" ${_${_PYTHON_PREFIX}_HINTS}
                                  PATH_SUFFIXES bin)
  endif()

  # Don't search for include dir if no library was founded
  if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE AND NOT _${_PYTHON_PREFIX}_INCLUDE_DIR)
    if ((${_PYTHON_PREFIX}_Interpreter_FOUND AND NOT CMAKE_CROSSCOMPILING) OR _${_PYTHON_PREFIX}_CONFIG)
      _python_get_config_var (_${_PYTHON_PREFIX}_INCLUDE_DIRS INCLUDES)

      find_path (_${_PYTHON_PREFIX}_INCLUDE_DIR
                 NAMES Python.h
                 HINTS ${_${_PYTHON_PREFIX}_INCLUDE_DIRS}
                 NO_SYSTEM_ENVIRONMENT_PATH
                 NO_CMAKE_SYSTEM_PATH)
    endif()

    # Rely on HINTS and standard paths if interpreter or config tool failed to locate artifacts
    if (NOT _${_PYTHON_PREFIX}_INCLUDE_DIR)
      unset (_${_PYTHON_PREFIX}_VIRTUALENV_PATHS)
      if (_${_PYTHON_PREFIX}_FIND_VIRTUALENV MATCHES "^(FIRST|ONLY)$")
        set (_${_PYTHON_PREFIX}_VIRTUALENV_PATHS ENV VIRTUAL_ENV ENV CONDA_PREFIX)
      endif()
      unset (_${_PYTHON_PREFIX}_INCLUDE_HINTS)

      # Use the library's install prefix as a hint
      if (${_${_PYTHON_PREFIX}_LIBRARY_RELEASE} MATCHES "^(.+/Frameworks/Python.framework/Versions/[0-9.]+)")
        list (APPEND _${_PYTHON_PREFIX}_INCLUDE_HINTS "${CMAKE_MATCH_1}")
      elseif (${_${_PYTHON_PREFIX}_LIBRARY_RELEASE} MATCHES "^(.+)/lib(64|32)?/python[0-9.]+/config")
        list (APPEND _${_PYTHON_PREFIX}_INCLUDE_HINTS "${CMAKE_MATCH_1}")
      elseif (DEFINED CMAKE_LIBRARY_ARCHITECTURE AND ${_${_PYTHON_PREFIX}_LIBRARY_RELEASE} MATCHES "^(.+)/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
        list (APPEND _${_PYTHON_PREFIX}_INCLUDE_HINTS "${CMAKE_MATCH_1}")
      else()
        # assume library is in a directory under root
        get_filename_component (_${_PYTHON_PREFIX}_PREFIX "${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}" DIRECTORY)
        get_filename_component (_${_PYTHON_PREFIX}_PREFIX "${_${_PYTHON_PREFIX}_PREFIX}" DIRECTORY)
        list (APPEND _${_PYTHON_PREFIX}_INCLUDE_HINTS "${_${_PYTHON_PREFIX}_PREFIX}")
      endif()

      _python_get_frameworks (_${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_VERSION})
      _python_get_registries (_${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_VERSION})
      _python_get_path_suffixes (_${_PYTHON_PREFIX}_PATH_SUFFIXES VERSION ${_${_PYTHON_PREFIX}_VERSION} INCLUDE)

      if (APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "FIRST")
        find_path (_${_PYTHON_PREFIX}_INCLUDE_DIR
                   NAMES Python.h
                   HINTS ${_${_PYTHON_PREFIX}_INCLUDE_HINTS} ${_${_PYTHON_PREFIX}_HINTS}
                   PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                         ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                   PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                   NO_CMAKE_PATH
                   NO_CMAKE_ENVIRONMENT_PATH
                   NO_SYSTEM_ENVIRONMENT_PATH
                   NO_CMAKE_SYSTEM_PATH)
      endif()

      if (WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "FIRST")
        find_path (_${_PYTHON_PREFIX}_INCLUDE_DIR
                   NAMES Python.h
                   HINTS ${_${_PYTHON_PREFIX}_INCLUDE_HINTS} ${_${_PYTHON_PREFIX}_HINTS}
                   PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                         ${_${_PYTHON_PREFIX}_REGISTRY_PATHS}
                   PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                   NO_SYSTEM_ENVIRONMENT_PATH
                   NO_CMAKE_SYSTEM_PATH)
      endif()

      if (APPLE AND _${_PYTHON_PREFIX}_FIND_FRAMEWORK STREQUAL "LAST")
        set (__${_PYTHON_PREFIX}_FRAMEWORK_PATHS ${_${_PYTHON_PREFIX}_FRAMEWORK_PATHS})
      else()
        unset (__${_PYTHON_PREFIX}_FRAMEWORK_PATHS)
      endif()

      if (WIN32 AND _${_PYTHON_PREFIX}_FIND_REGISTRY STREQUAL "LAST")
        set (__${_PYTHON_PREFIX}_REGISTRY_PATHS ${_${_PYTHON_PREFIX}_REGISTRY_PATHS})
      else()
        unset (__${_PYTHON_PREFIX}_REGISTRY_PATHS)
      endif()

      find_path (_${_PYTHON_PREFIX}_INCLUDE_DIR
                 NAMES Python.h
                 HINTS ${_${_PYTHON_PREFIX}_INCLUDE_HINTS} ${_${_PYTHON_PREFIX}_HINTS}
                 PATHS ${_${_PYTHON_PREFIX}_VIRTUALENV_PATHS}
                       ${__${_PYTHON_PREFIX}_FRAMEWORK_PATHS}
                       ${__${_PYTHON_PREFIX}_REGISTRY_PATHS}
                 PATH_SUFFIXES ${_${_PYTHON_PREFIX}_PATH_SUFFIXES}
                 NO_SYSTEM_ENVIRONMENT_PATH
                 NO_CMAKE_SYSTEM_PATH)
    endif()

    # search header file in standard locations
    find_path (_${_PYTHON_PREFIX}_INCLUDE_DIR
               NAMES Python.h)
  endif()

  set (${_PYTHON_PREFIX}_INCLUDE_DIRS "${_${_PYTHON_PREFIX}_INCLUDE_DIR}")

  if (_${_PYTHON_PREFIX}_INCLUDE_DIR AND NOT EXISTS "${_${_PYTHON_PREFIX}_INCLUDE_DIR}")
    set (_${_PYTHON_PREFIX}_Development_REASON_FAILURE "Cannot find the directory \"${_${_PYTHON_PREFIX}_INCLUDE_DIR}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_INCLUDE_DIR-NOTFOUND")
  endif()

  if (_${_PYTHON_PREFIX}_INCLUDE_DIR)
    # retrieve version from header file
    _python_get_version (INCLUDE PREFIX _${_PYTHON_PREFIX}_INC_)

    # update versioning
    if (_${_PYTHON_PREFIX}_INC_VERSION VERSION_EQUAL ${_${_PYTHON_PREFIX}_VERSION})
      set (_${_PYTHON_PREFIX}_VERSION_PATCH ${_${_PYTHON_PREFIX}_INC_VERSION_PATCH})
    endif()
  endif()

  if (NOT ${_PYTHON_PREFIX}_Interpreter_FOUND AND NOT ${_PYTHON_PREFIX}_Compiler_FOUND)
    # set public version information
    set (${_PYTHON_PREFIX}_VERSION ${_${_PYTHON_PREFIX}_VERSION})
    set (${_PYTHON_PREFIX}_VERSION_MAJOR ${_${_PYTHON_PREFIX}_VERSION_MAJOR})
    set (${_PYTHON_PREFIX}_VERSION_MINOR ${_${_PYTHON_PREFIX}_VERSION_MINOR})
    set (${_PYTHON_PREFIX}_VERSION_PATCH ${_${_PYTHON_PREFIX}_VERSION_PATCH})
  endif()

  # define public variables
  set (${_PYTHON_PREFIX}_LIBRARY_DEBUG "${_${_PYTHON_PREFIX}_LIBRARY_DEBUG}")
  _python_select_library_configurations (${_PYTHON_PREFIX})

  set (${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE "${_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE}")
  set (${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG "${_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG}")

  if (_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE)
    set (${_PYTHON_PREFIX}_RUNTIME_LIBRARY "${_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE}")
  elseif (_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG)
    set (${_PYTHON_PREFIX}_RUNTIME_LIBRARY "${_${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG}")
  else()
    set (${_PYTHON_PREFIX}_RUNTIME_LIBRARY "${_PYTHON_PREFIX}_RUNTIME_LIBRARY-NOTFOUND")
  endif()

  _python_set_library_dirs (${_PYTHON_PREFIX}_LIBRARY_DIRS
                            _${_PYTHON_PREFIX}_LIBRARY_RELEASE _${_PYTHON_PREFIX}_LIBRARY_DEBUG)
  if (UNIX)
    if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE MATCHES "${CMAKE_SHARED_LIBRARY_SUFFIX}$")
      set (${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DIRS ${${_PYTHON_PREFIX}_LIBRARY_DIRS})
    endif()
  else()
      _python_set_library_dirs (${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DIRS
                                _${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE _${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG)
  endif()

  if (_${_PYTHON_PREFIX}_LIBRARY_RELEASE AND _${_PYTHON_PREFIX}_INCLUDE_DIR)
    if (${_PYTHON_PREFIX}_Interpreter_FOUND OR ${_PYTHON_PREFIX}_Compiler_FOUND)
      # development environment must be compatible with interpreter/compiler
      if (${_${_PYTHON_PREFIX}_VERSION_MAJOR}.${_${_PYTHON_PREFIX}_VERSION_MINOR} VERSION_EQUAL ${${_PYTHON_PREFIX}_VERSION_MAJOR}.${${_PYTHON_PREFIX}_VERSION_MINOR}
          AND ${_${_PYTHON_PREFIX}_INC_VERSION_MAJOR}.${_${_PYTHON_PREFIX}_INC_VERSION_MINOR} VERSION_EQUAL ${_${_PYTHON_PREFIX}_VERSION_MAJOR}.${_${_PYTHON_PREFIX}_VERSION_MINOR})
        set (${_PYTHON_PREFIX}_Development_FOUND TRUE)
      endif()
    elseif (${_PYTHON_PREFIX}_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR
            AND ${_${_PYTHON_PREFIX}_INC_VERSION_MAJOR}.${_${_PYTHON_PREFIX}_INC_VERSION_MINOR} VERSION_EQUAL ${_${_PYTHON_PREFIX}_VERSION_MAJOR}.${_${_PYTHON_PREFIX}_VERSION_MINOR})
      set (${_PYTHON_PREFIX}_Development_FOUND TRUE)
    endif()
    if (DEFINED _${_PYTHON_PREFIX}_FIND_ABI AND
        (NOT _${_PYTHON_PREFIX}_ABI IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS
          OR NOT _${_PYTHON_PREFIX}_INC_ABI IN_LIST _${_PYTHON_PREFIX}_ABIFLAGS))
      set (${_PYTHON_PREFIX}_Development_FOUND FALSE)
    endif()
  endif()

  if (_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR VERSION_GREATER_EQUAL 3
      AND NOT DEFINED ${_PYTHON_PREFIX}_SOABI)
    _python_get_config_var (${_PYTHON_PREFIX}_SOABI SOABI)
  endif()

  if (${_PYTHON_PREFIX}_Development_FOUND)
    # compute and save development signature
    string (MD5 __${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE "${_${_PYTHON_PREFIX}_SIGNATURE}:${_${_PYTHON_PREFIX}_LIBRARY_RELEASE}:${_${_PYTHON_PREFIX}_INCLUDE_DIR}")
    set (_${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE "${__${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE}" CACHE INTERNAL "")
  else()
    unset (_${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE CACHE)
  endif()

  # Restore the original find library ordering
  if (DEFINED _${_PYTHON_PREFIX}_CMAKE_FIND_LIBRARY_SUFFIXES)
    set (CMAKE_FIND_LIBRARY_SUFFIXES ${_${_PYTHON_PREFIX}_CMAKE_FIND_LIBRARY_SUFFIXES})
  endif()

  _python_mark_as_internal (_${_PYTHON_PREFIX}_LIBRARY_RELEASE
                            _${_PYTHON_PREFIX}_LIBRARY_DEBUG
                            _${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE
                            _${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG
                            _${_PYTHON_PREFIX}_INCLUDE_DIR
                            _${_PYTHON_PREFIX}_CONFIG
                            _${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE)
endif()

if ("NumPy" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS AND ${_PYTHON_PREFIX}_Interpreter_FOUND)
  list (APPEND _${_PYTHON_PREFIX}_CACHED_VARS _${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR)
  if (${_PYTHON_PREFIX}_FIND_REQUIRED_NumPy)
    list (APPEND _${_PYTHON_PREFIX}_REQUIRED_VARS ${_PYTHON_PREFIX}_NumPy_INCLUDE_DIRS)
  endif()

  if (DEFINED ${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR
      AND IS_ABSOLUTE "${${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}")
    set (_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR "${${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}" CACHE INTERNAL "")
  elseif (DEFINED _${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR)
    # compute numpy signature. Depends on interpreter and development signatures
    string (MD5 __${_PYTHON_PREFIX}_NUMPY_SIGNATURE "${_${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE}:${_${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE}:${_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}")
    if (NOT __${_PYTHON_PREFIX}_NUMPY_SIGNATURE STREQUAL _${_PYTHON_PREFIX}_NUMPY_SIGNATURE
        OR NOT EXISTS "${_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}")
      unset (_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR CACHE)
      unset (_${_PYTHON_PREFIX}_NUMPY_SIGNATURE CACHE)
    endif()
  endif()

  if (NOT _${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR)
    execute_process(
      COMMAND "${${_PYTHON_PREFIX}_EXECUTABLE}" -c
              "from __future__ import print_function\ntry: import numpy; print(numpy.get_include(), end='')\nexcept:pass\n"
      RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
      OUTPUT_VARIABLE _${_PYTHON_PREFIX}_NumPy_PATH
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (NOT _${_PYTHON_PREFIX}_RESULT)
      find_path (_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR
                 NAMES "numpy/arrayobject.h" "numpy/numpyconfig.h"
                 HINTS "${_${_PYTHON_PREFIX}_NumPy_PATH}"
                 NO_DEFAULT_PATH)
    endif()
  endif()

  set (${_PYTHON_PREFIX}_NumPy_INCLUDE_DIRS "${_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}")

  if(_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR AND NOT EXISTS "${_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}")
    set (_${_PYTHON_PREFIX}_NumPy_REASON_FAILURE "Cannot find the directory \"${_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR}\"")
    set_property (CACHE _${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR PROPERTY VALUE "${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR-NOTFOUND")
  endif()

  if (_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR)
    execute_process (
            COMMAND "${${_PYTHON_PREFIX}_EXECUTABLE}" -c
            "from __future__ import print_function\ntry: import numpy; print(numpy.__version__, end='')\nexcept:pass\n"
            RESULT_VARIABLE _${_PYTHON_PREFIX}_RESULT
            OUTPUT_VARIABLE _${_PYTHON_PREFIX}_NumPy_VERSION)
    if (NOT _${_PYTHON_PREFIX}_RESULT)
      set (${_PYTHON_PREFIX}_NumPy_VERSION "${_${_PYTHON_PREFIX}_NumPy_VERSION}")
    else()
      unset (${_PYTHON_PREFIX}_NumPy_VERSION)
    endif()

    # final step: set NumPy founded only if Development component is founded as well
    set(${_PYTHON_PREFIX}_NumPy_FOUND ${${_PYTHON_PREFIX}_Development_FOUND})
  else()
    set (${_PYTHON_PREFIX}_NumPy_FOUND FALSE)
  endif()

  if (${_PYTHON_PREFIX}_NumPy_FOUND)
    # compute and save numpy signature
    string (MD5 __${_PYTHON_PREFIX}_NUMPY_SIGNATURE "${_${_PYTHON_PREFIX}_INTERPRETER_SIGNATURE}:${_${_PYTHON_PREFIX}_DEVELOPMENT_SIGNATURE}:${${_PYTHON_PREFIX}_NumPyINCLUDE_DIR}")
    set (_${_PYTHON_PREFIX}_NUMPY_SIGNATURE "${__${_PYTHON_PREFIX}_NUMPY_SIGNATURE}" CACHE INTERNAL "")
  else()
    unset (_${_PYTHON_PREFIX}_NUMPY_SIGNATURE CACHE)
  endif()

  _python_mark_as_internal (_${_PYTHON_PREFIX}_NumPy_INCLUDE_DIR
                            _${_PYTHON_PREFIX}_NUMPY_SIGNATURE)
endif()

# final validation
if (${_PYTHON_PREFIX}_VERSION_MAJOR AND
    NOT ${_PYTHON_PREFIX}_VERSION_MAJOR VERSION_EQUAL _${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR)
  _python_display_failure ("Could NOT find ${_PYTHON_PREFIX}: Found unsuitable major version \"${${_PYTHON_PREFIX}_VERSION_MAJOR}\", but required major version is exact version \"${_${_PYTHON_PREFIX}_REQUIRED_VERSION_MAJOR}\"")
endif()

unset (_${_PYTHON_PREFIX}_REASON_FAILURE)
foreach (_${_PYTHON_PREFIX}_COMPONENT IN ITEMS Interpreter Compiler Development NumPy)
  if (_${_PYTHON_PREFIX}_${_${_PYTHON_PREFIX}_COMPONENT}_REASON_FAILURE)
    string (APPEND _${_PYTHON_PREFIX}_REASON_FAILURE "\n        ${_${_PYTHON_PREFIX}_COMPONENT}: ${_${_PYTHON_PREFIX}_${_${_PYTHON_PREFIX}_COMPONENT}_REASON_FAILURE}")
  endif()
endforeach()

include (${CMAKE_CURRENT_LIST_DIR}/../FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args (${_PYTHON_PREFIX}
                                   REQUIRED_VARS ${_${_PYTHON_PREFIX}_REQUIRED_VARS}
                                   VERSION_VAR ${_PYTHON_PREFIX}_VERSION
                                   HANDLE_COMPONENTS
                                   REASON_FAILURE_MESSAGE "${_${_PYTHON_PREFIX}_REASON_FAILURE}")

# Create imported targets and helper functions
if(_${_PYTHON_PREFIX}_CMAKE_ROLE STREQUAL "PROJECT")
  if ("Interpreter" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS
      AND ${_PYTHON_PREFIX}_Interpreter_FOUND
      AND NOT TARGET ${_PYTHON_PREFIX}::Interpreter)
    add_executable (${_PYTHON_PREFIX}::Interpreter IMPORTED)
    set_property (TARGET ${_PYTHON_PREFIX}::Interpreter
                  PROPERTY IMPORTED_LOCATION "${${_PYTHON_PREFIX}_EXECUTABLE}")
  endif()

  if ("Compiler" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS
      AND ${_PYTHON_PREFIX}_Compiler_FOUND
      AND NOT TARGET ${_PYTHON_PREFIX}::Compiler)
    add_executable (${_PYTHON_PREFIX}::Compiler IMPORTED)
    set_property (TARGET ${_PYTHON_PREFIX}::Compiler
                  PROPERTY IMPORTED_LOCATION "${${_PYTHON_PREFIX}_COMPILER}")
  endif()

  if ("Development" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS
      AND ${_PYTHON_PREFIX}_Development_FOUND)

    macro (__PYTHON_IMPORT_LIBRARY __name)
      if (${_PYTHON_PREFIX}_LIBRARY_RELEASE MATCHES "${CMAKE_SHARED_LIBRARY_SUFFIX}$"
          OR ${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE)
        set (_${_PYTHON_PREFIX}_LIBRARY_TYPE SHARED)
      else()
        set (_${_PYTHON_PREFIX}_LIBRARY_TYPE STATIC)
      endif()

      if (NOT TARGET ${__name})
        add_library (${__name} ${_${_PYTHON_PREFIX}_LIBRARY_TYPE} IMPORTED)
      endif()

      set_property (TARGET ${__name}
                    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${_PYTHON_PREFIX}_INCLUDE_DIRS}")

      if (${_PYTHON_PREFIX}_LIBRARY_RELEASE AND ${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE)
        # System manage shared libraries in two parts: import and runtime
        if (${_PYTHON_PREFIX}_LIBRARY_RELEASE AND ${_PYTHON_PREFIX}_LIBRARY_DEBUG)
          set_property (TARGET ${__name} PROPERTY IMPORTED_CONFIGURATIONS RELEASE DEBUG)
          set_target_properties (${__name}
                                 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
                                            IMPORTED_IMPLIB_RELEASE "${${_PYTHON_PREFIX}_LIBRARY_RELEASE}"
                                            IMPORTED_LOCATION_RELEASE "${${_PYTHON_PREFIX}_RUNTIME_LIBRARY_RELEASE}")
          set_target_properties (${__name}
                                 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
                                            IMPORTED_IMPLIB_DEBUG "${${_PYTHON_PREFIX}_LIBRARY_DEBUG}"
                                            IMPORTED_LOCATION_DEBUG "${${_PYTHON_PREFIX}_RUNTIME_LIBRARY_DEBUG}")
        else()
          set_target_properties (${__name}
                                 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                            IMPORTED_IMPLIB "${${_PYTHON_PREFIX}_LIBRARY}"
                                            IMPORTED_LOCATION "${${_PYTHON_PREFIX}_RUNTIME_LIBRARY}")
        endif()
      else()
        if (${_PYTHON_PREFIX}_LIBRARY_RELEASE AND ${_PYTHON_PREFIX}_LIBRARY_DEBUG)
          set_property (TARGET ${__name} PROPERTY IMPORTED_CONFIGURATIONS RELEASE DEBUG)
          set_target_properties (${__name}
                                 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
                                            IMPORTED_LOCATION_RELEASE "${${_PYTHON_PREFIX}_LIBRARY_RELEASE}")
          set_target_properties (${__name}
                                 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
                                            IMPORTED_LOCATION_DEBUG "${${_PYTHON_PREFIX}_LIBRARY_DEBUG}")
        else()
          set_target_properties (${__name}
                                 PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                                            IMPORTED_LOCATION "${${_PYTHON_PREFIX}_LIBRARY}")
        endif()
      endif()

      if (_${_PYTHON_PREFIX}_LIBRARY_TYPE STREQUAL "STATIC")
        # extend link information with dependent libraries
        _python_get_config_var (_${_PYTHON_PREFIX}_LINK_LIBRARIES LIBS)
        if (_${_PYTHON_PREFIX}_LINK_LIBRARIES)
          set_property (TARGET ${__name}
                        PROPERTY INTERFACE_LINK_LIBRARIES ${_${_PYTHON_PREFIX}_LINK_LIBRARIES})
        endif()
      endif()
    endmacro()

    __python_import_library (${_PYTHON_PREFIX}::Python)

    if (CMAKE_SYSTEM_NAME MATCHES "^(Windows.*|CYGWIN|MSYS)$")
      # On Windows/CYGWIN/MSYS, Python::Module is the same as Python::Python
      # but ALIAS cannot be used because the imported library is not GLOBAL.
      __python_import_library (${_PYTHON_PREFIX}::Module)
    else()
      if (NOT TARGET ${_PYTHON_PREFIX}::Module )
        add_library (${_PYTHON_PREFIX}::Module INTERFACE IMPORTED)
      endif()
      set_property (TARGET ${_PYTHON_PREFIX}::Module
                    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${_PYTHON_PREFIX}_INCLUDE_DIRS}")

      # When available, enforce shared library generation with undefined symbols
      if (APPLE)
        set_property (TARGET ${_PYTHON_PREFIX}::Module
                      PROPERTY INTERFACE_LINK_OPTIONS "LINKER:-undefined,dynamic_lookup")
      endif()
      if (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
        set_property (TARGET ${_PYTHON_PREFIX}::Module
                      PROPERTY INTERFACE_LINK_OPTIONS "LINKER:-z,nodefs")
      endif()
      if (CMAKE_SYSTEM_NAME STREQUAL "AIX")
        set_property (TARGET ${_PYTHON_PREFIX}::Module
                      PROPERTY INTERFACE_LINK_OPTIONS "LINKER:-b,erok")
      endif()
    endif()

    #
    # PYTHON_ADD_LIBRARY (<name> [STATIC|SHARED|MODULE] src1 src2 ... srcN)
    # It is used to build modules for python.
    #
    function (__${_PYTHON_PREFIX}_ADD_LIBRARY prefix name)
      cmake_parse_arguments (PARSE_ARGV 2 PYTHON_ADD_LIBRARY
        "STATIC;SHARED;MODULE;WITH_SOABI" "" "")

      if (prefix STREQUAL "Python2" AND PYTHON_ADD_LIBRARY_WITH_SOABI)
        message (AUTHOR_WARNING "FindPython2: Option `WITH_SOABI` is not supported for Python2 and will be ignored.")
        unset (PYTHON_ADD_LIBRARY_WITH_SOABI)
      endif()

      if (PYTHON_ADD_LIBRARY_STATIC)
        set (type STATIC)
      elseif (PYTHON_ADD_LIBRARY_SHARED)
        set (type SHARED)
      else()
        set (type MODULE)
      endif()
      add_library (${name} ${type} ${PYTHON_ADD_LIBRARY_UNPARSED_ARGUMENTS})

      get_property (type TARGET ${name} PROPERTY TYPE)

      if (type STREQUAL "MODULE_LIBRARY")
        target_link_libraries (${name} PRIVATE ${prefix}::Module)
        # customize library name to follow module name rules
        set_property (TARGET ${name} PROPERTY PREFIX "")
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
          set_property (TARGET ${name} PROPERTY SUFFIX ".pyd")
        endif()

        if (PYTHON_ADD_LIBRARY_WITH_SOABI AND ${prefix}_SOABI)
          get_property (suffix TARGET ${name} PROPERTY SUFFIX)
          if (NOT suffix)
            set (suffix "${CMAKE_SHARED_MODULE_SUFFIX}")
          endif()
          set_property (TARGET ${name} PROPERTY SUFFIX ".${${prefix}_SOABI}${suffix}")
        endif()
      else()
        if (PYTHON_ADD_LIBRARY_WITH_SOABI)
          message (AUTHOR_WARNING "Find${prefix}: Option `WITH_SOABI` is only supported for `MODULE` library type.")
        endif()
        target_link_libraries (${name} PRIVATE ${prefix}::Python)
      endif()
    endfunction()
  endif()

  if ("NumPy" IN_LIST ${_PYTHON_PREFIX}_FIND_COMPONENTS AND ${_PYTHON_PREFIX}_NumPy_FOUND
      AND NOT TARGET ${_PYTHON_PREFIX}::NumPy AND TARGET ${_PYTHON_PREFIX}::Module)
    add_library (${_PYTHON_PREFIX}::NumPy INTERFACE IMPORTED)
    set_property (TARGET ${_PYTHON_PREFIX}::NumPy
                  PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${_PYTHON_PREFIX}_NumPy_INCLUDE_DIRS}")
    target_link_libraries (${_PYTHON_PREFIX}::NumPy INTERFACE ${_PYTHON_PREFIX}::Module)
  endif()
endif()

# final clean-up

# Restore CMAKE_FIND_APPBUNDLE
if (DEFINED _${_PYTHON_PREFIX}_CMAKE_FIND_APPBUNDLE)
  set (CMAKE_FIND_APPBUNDLE ${_${_PYTHON_PREFIX}_CMAKE_FIND_APPBUNDLE})
  unset (_${_PYTHON_PREFIX}_CMAKE_FIND_APPBUNDLE)
else()
  unset (CMAKE_FIND_APPBUNDLE)
endif()
# Restore CMAKE_FIND_FRAMEWORK
if (DEFINED _${_PYTHON_PREFIX}_CMAKE_FIND_FRAMEWORK)
  set (CMAKE_FIND_FRAMEWORK ${_${_PYTHON_PREFIX}_CMAKE_FIND_FRAMEWORK})
  unset (_${_PYTHON_PREFIX}_CMAKE_FIND_FRAMEWORK)
else()
  unset (CMAKE_FIND_FRAMEWORK)
endif()
