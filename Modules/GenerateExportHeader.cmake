
# - Function for generation of export macros for libraries
#
# This module provides the function GENERATE_EXPORT_HEADER() and the
# accompanying ADD_COMPILER_EXPORT_FLAGS() function.
#
# The GENERATE_EXPORT_HEADER function can be used to generate a file suitable
# for preprocessor inclusion which contains EXPORT macros to be used in
# library classes.
#
# GENERATE_EXPORT_HEADER( LIBRARY_TARGET
#             [BASE_NAME <base_name>]
#             [EXPORT_MACRO_NAME <export_macro_name>]
#             [EXPORT_FILE_NAME <export_file_name>]
#             [DEPRECATED_NAME <deprecated_name>]
#             [NO_EXPORT_MACRO_NAME <no_export_macro_name>]
#             [STATIC_DEFINE <static_define>]
# )
#
# ADD_COMPILER_EXPORT_FLAGS( [FATAL_WARNINGS] )
#
# By default GENERATE_EXPORT_HEADER() generates macro names in a file name
# determined by the name of the library. The ADD_COMPILER_EXPORT_FLAGS macro adds
# -fvisibility=hidden to CMAKE_CXX_FLAGS if supported, and is a no-op on Windows
# which does not need extra compiler flags for exporting support.
#
# This means that in the simplest case, users of these functions will be equivalent to:
#
#   add_compiler_export_flags()
#
#   add_library(somelib someclass.cpp)
#
#   generate_export_header(somelib)
#
#   install(TARGETS somelib DESTINATION ${LIBRARY_INSTALL_DIR})
#
#   install(FILES
#    someclass.h
#    ${PROJECT_BINARY_DIR}/somelib_export.h DESTINATION ${INCLUDE_INSTALL_DIR}
#   )
#
# And in the ABI header files:
#
#   \code
#   #include "somelib_export.h"
#
#   class SOMELIB_EXPORT SomeClass {
#
#   };
#   \endcode
#
# The CMake fragment will generate a file in the ${CMAKE_CURRENT_BUILD_DIR} called
# somelib_export.h containing the macros SOMELIB_EXPORT, SOMELIB_NO_EXPORT,
# SOMELIB_DEPRECATED, SOMELIB_DEPRECATED_EXPORT and SOMELIB_DEPRECATED_NO_EXPORT.
# The resulting file should be installed with other headers in the library.
#
# The BASE_NAME argument can be used to override the file name and the names
# used for the macros
#
#   add_library(somelib someclass.cpp)
#   generate_export_header(somelib
#     BASE_NAME other_name
#   )
#
# Generates a file called other_name_export.h containing the macros
# OTHER_NAME_EXPORT, OTHER_NAME_NO_EXPORT and OTHER_NAME_DEPRECATED etc.
#
# The BASE_NAME may be overridden by specifiying other options in the function.
# For example:
#
#   add_library(somelib someclass.cpp)
#   generate_export_header(somelib
#     EXPORT_MACRO_NAME OTHER_NAME_EXPORT
#   )
#
# creates the macro OTHER_NAME_EXPORT instead of SOMELIB_EXPORT, but other macros
# and the generated file name is as default.
#
#   add_library(somelib someclass.cpp)
#   generate_export_header(somelib
#     DEPRECATED_NAME KDE_DEPRECATED
#   )
#
# creates the macro KDE_DEPRECATED instead of SOMELIB_DEPRECATED.
#
# If LIBRARY_TARGET is a static library, macros are defined without values.
#
# If the same sources are used to create both a shared and a static library, the
# uppercased symbol ${BASE_NAME}_STATIC_DEFINE should be used when building the
# static library
#
#   add_library(shared_variant SHARED ${lib_SRCS})
#   add_library(static_variant ${lib_SRCS})
#
#   generate_export_header(shared_variant BASE_NAME libshared_and_static)
#
#   set_target_properties(static_variant PROPERTIES COMPILE_FLAGS -DLIBSHARED_AND_STATIC_STATIC_DEFINE)
#
# This will cause the export macros to expand to nothing when building the static library.

include(CMakeParseArguments)
include(CheckCXXCompilerFlag)

macro(_test_compiler_hidden_visibility)
  check_cxx_compiler_flag(-fvisibility=hidden COMPILER_HAS_HIDDEN_VISIBILITY)
  check_cxx_compiler_flag(-fvisibility-inlines-hidden COMPILER_HAS_HIDDEN_INLINE_VISIBILITY)
  option(USE_COMPILER_HIDDEN_VISIBILITY "Use HIDDEN visibility support if available." ON)
  mark_as_advanced(USE_COMPILER_HIDDEN_VISIBILITY)
endmacro()

set(myDir ${CMAKE_CURRENT_LIST_DIR})

macro(_DO_SET_MACRO_VALUES TARGET_LIBRARY)
  set(DEFINE_DEPRECATED)
  set(DEFINE_EXPORT)
  set(DEFINE_IMPORT)
  set(DEFINE_NO_EXPORT)

  if(WIN32)
    set(DEFINE_DEPRECATED "__declspec(deprecated)")
  else()
    set(DEFINE_DEPRECATED "__attribute__ ((__deprecated__))")
  endif()

  get_property(type TARGET ${TARGET_LIBRARY} PROPERTY TYPE)

  if(NOT ${type} STREQUAL "STATIC_LIBRARY")
    if(WIN32)
      set(DEFINE_EXPORT "__declspec(dllexport)")
      set(DEFINE_IMPORT "__declspec(dllimport)")
    elseif(CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES Intel AND UNIX))
      if(COMPILER_HAS_HIDDEN_VISIBILITY AND USE_COMPILER_HIDDEN_VISIBILITY)
        set(DEFINE_EXPORT "__attribute__((visibility(\"default\")))")
        set(DEFINE_IMPORT "__attribute__((visibility(\"default\")))")
        set(DEFINE_NO_EXPORT "__attribute__((visibility(\"hidden\")))")
      endif()
    endif()
  endif()
endmacro()

macro(_DO_GENERATE_EXPORT_HEADER TARGET_LIBRARY)
  # Option overrides
  set(options)
  set(oneValueArgs PREFIX_NAME BASE_NAME EXPORT_MACRO_NAME EXPORT_FILE_NAME DEPRECATED_NAME NO_EXPORT_MACRO_NAME STATIC_DEFINE)
  set(multiValueArgs)

  cmake_parse_arguments(_GEH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(BASE_NAME "${TARGET_LIBRARY}")

  if(_GEH_BASE_NAME)
    set(BASE_NAME ${_GEH_BASE_NAME})
  endif()

  string(TOUPPER ${BASE_NAME} BASE_NAME_UPPER)
  string(TOLOWER ${BASE_NAME} BASE_NAME_LOWER)

  # Default options
  set(EXPORT_MACRO_NAME "${PREFIX}${BASE_NAME_UPPER}_EXPORT")
  set(NO_EXPORT_MACRO_NAME "${PREFIX}${BASE_NAME_UPPER}_NO_EXPORT")
  set(EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME_LOWER}_export.h")
  set(DEPRECATED_NAME "${PREFIX}${BASE_NAME_UPPER}_DEPRECATED")
  set(STATIC_DEFINE "${PREFIX}${BASE_NAME_UPPER}_STATIC_DEFINE")

  if(_GEH_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to GENERATE_EXPORT_HEADER(): \"${_GEH_UNPARSED_ARGUMENTS}\"")
  endif()

  if(_GEH_EXPORT_MACRO_NAME)
    set(EXPORT_MACRO_NAME ${PREFIX}${_GEH_EXPORT_MACRO_NAME})
  endif()
  if(_GEH_EXPORT_FILE_NAME)
    if(IS_ABSOLUTE _GEH_EXPORT_FILE_NAME)
      set(EXPORT_FILE_NAME ${_GEH_EXPORT_FILE_NAME})
    else()
      set(EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${_GEH_EXPORT_FILE_NAME}")
    endif()
  endif()
  if(_GEH_DEPRECATED_NAME)
    set(DEPRECATED_NAME ${PREFIX}${_GEH_DEPRECATED_NAME})
  endif()
  if(_GEH_NO_EXPORT_MACRO_NAME)
    set(NO_EXPORT_MACRO_NAME ${PREFIX}${_GEH_NO_EXPORT_MACRO_NAME})
  endif()
  if(_GEH_STATIC_DEFINE)
    set(STATIC_DEFINE ${PREFIX}${_GEH_STATIC_DEFINE})
  endif()

  set(INCLUDE_GUARD_NAME "${PREFIX}${EXPORT_MACRO_NAME}_H")

  get_target_property(EXPORT_IMPORT_CONDITION ${TARGET_LIBRARY} DEFINE_SYMBOL)

  if (NOT EXPORT_IMPORT_CONDITION)
    set(EXPORT_IMPORT_CONDITION ${TARGET_LIBRARY}_EXPORTS)
  endif()

  configure_file(${myDir}/exportheader.cmake.in ${EXPORT_FILE_NAME} @ONLY)
endmacro()

function(GENERATE_EXPORT_HEADER TARGET_LIBRARY)
  get_property(type TARGET ${TARGET_LIBRARY} PROPERTY TYPE)
  if(${type} STREQUAL "MODULE")
    message(WARNING "This macro should not be used with libraries of type MODULE")
    return()
  endif()
  if(NOT ${type} STREQUAL "STATIC_LIBRARY" AND NOT ${type} STREQUAL "SHARED_LIBRARY")
    message(WARNING "This macro can only be used with libraries")
    return()
  endif()
  _test_compiler_hidden_visibility()
  _do_set_macro_values(${TARGET_LIBRARY})
  _do_generate_export_header(${TARGET_LIBRARY} ${ARGN})
endfunction()

function(add_compiler_export_flags)

  if(NOT CMAKE_COMPILER_IS_GNUCXX OR MINGW)
    return()
  endif()

  set(options)
  set(oneValueArgs FATAL_WARNINGS)
  set(multiValueArgs)

  cmake_parse_arguments(_EGHV "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(MESSAGE_TYPE WARNING)

  if(_EGHV_FATAL_WARNINGS)
    set(MESSAGE_TYPE FATAL_ERROR)
  endif()

  if (CMAKE_COMPILER_IS_GNUCXX)
    exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE     _gcc_version_info)
    string (REGEX MATCH "[345]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
    # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the
    # patch level, handle this here:
    if(NOT _gcc_version)
      string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
    endif()

    if(${_gcc_version} VERSION_LESS "4.2")
      return()
    endif()
  endif()

  _test_compiler_hidden_visibility()

  if(USE_COMPILER_HIDDEN_VISIBILITY AND COMPILER_HAS_HIDDEN_VISIBILITY AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
    set (EXTRA_FLAGS "-fvisibility=hidden")

    if(COMPILER_HAS_HIDDEN_INLINE_VISIBILITY)
      set (EXTRA_FLAGS "${EXTRA_FLAGS} -fvisibility-inlines-hidden")
    endif()
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_FLAGS}" PARENT_SCOPE)
endfunction()