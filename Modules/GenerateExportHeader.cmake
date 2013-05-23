# - Function for generation of export macros for libraries
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
#             [DEPRECATED_MACRO_NAME <deprecated_macro_name>]
#             [NO_EXPORT_MACRO_NAME <no_export_macro_name>]
#             [STATIC_DEFINE <static_define>]
#             [NO_DEPRECATED_MACRO_NAME <no_deprecated_macro_name>]
#             [DEFINE_NO_DEPRECATED]
#             [PREFIX_NAME <prefix_name>]
# )
#
# ADD_COMPILER_EXPORT_FLAGS( [<output_variable>] )
#
# By default GENERATE_EXPORT_HEADER() generates macro names in a file name
# determined by the name of the library. The ADD_COMPILER_EXPORT_FLAGS function
# adds -fvisibility=hidden to CMAKE_CXX_FLAGS if supported, and is a no-op on
# Windows which does not need extra compiler flags for exporting support. You
# may optionally pass a single argument to ADD_COMPILER_EXPORT_FLAGS that will
# be populated with the required CXX_FLAGS required to enable visibility support
# for the compiler/architecture in use.
#
# This means that in the simplest case, users of these functions will be
# equivalent to:
#
#   add_compiler_export_flags()
#   add_library(somelib someclass.cpp)
#   generate_export_header(somelib)
#   install(TARGETS somelib DESTINATION ${LIBRARY_INSTALL_DIR})
#   install(FILES
#    someclass.h
#    ${PROJECT_BINARY_DIR}/somelib_export.h DESTINATION ${INCLUDE_INSTALL_DIR}
#   )
#
# And in the ABI header files:
#
#   #include "somelib_export.h"
#   class SOMELIB_EXPORT SomeClass {
#     ...
#   };
#
# The CMake fragment will generate a file in the ${CMAKE_CURRENT_BINARY_DIR}
# called somelib_export.h containing the macros SOMELIB_EXPORT, SOMELIB_NO_EXPORT,
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
#     DEPRECATED_MACRO_NAME KDE_DEPRECATED
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
#   generate_export_header(shared_variant BASE_NAME libshared_and_static)
#   set_target_properties(static_variant PROPERTIES
#     COMPILE_FLAGS -DLIBSHARED_AND_STATIC_STATIC_DEFINE)
#
# This will cause the export macros to expand to nothing when building the
# static library.
#
# If DEFINE_NO_DEPRECATED is specified, then a macro ${BASE_NAME}_NO_DEPRECATED
# will be defined
# This macro can be used to remove deprecated code from preprocessor output.
#
#   option(EXCLUDE_DEPRECATED "Exclude deprecated parts of the library" FALSE)
#   if (EXCLUDE_DEPRECATED)
#     set(NO_BUILD_DEPRECATED DEFINE_NO_DEPRECATED)
#   endif()
#   generate_export_header(somelib ${NO_BUILD_DEPRECATED})
#
# And then in somelib:
#
#   class SOMELIB_EXPORT SomeClass
#   {
#   public:
#   #ifndef SOMELIB_NO_DEPRECATED
#     SOMELIB_DEPRECATED void oldMethod();
#   #endif
#   };
#
#   #ifndef SOMELIB_NO_DEPRECATED
#   void SomeClass::oldMethod() {  }
#   #endif
#
# If PREFIX_NAME is specified, the argument will be used as a prefix to all
# generated macros.
#
# For example:
#
#   generate_export_header(somelib PREFIX_NAME VTK_)
#
# Generates the macros VTK_SOMELIB_EXPORT etc.

#=============================================================================
# Copyright 2011 Stephen Kelly <steveire@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

include(CMakeParseArguments)
include(CheckCXXCompilerFlag)

# TODO: Install this macro separately?
macro(_check_cxx_compiler_attribute _ATTRIBUTE _RESULT)
  check_cxx_source_compiles("${_ATTRIBUTE} int somefunc() { return 0; }
    int main() { return somefunc();}" ${_RESULT}
  )
endmacro()

macro(_test_compiler_hidden_visibility)

  if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.2")
    set(GCC_TOO_OLD TRUE)
  elseif(CMAKE_COMPILER_IS_GNUC AND CMAKE_C_COMPILER_VERSION VERSION_LESS "4.2")
    set(GCC_TOO_OLD TRUE)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES Intel AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
    set(_INTEL_TOO_OLD TRUE)
  endif()

  # Exclude XL here because it misinterprets -fvisibility=hidden even though
  # the check_cxx_compiler_flag passes
  # http://www.cdash.org/CDash/testDetails.php?test=109109951&build=1419259
  if(NOT GCC_TOO_OLD
      AND NOT _INTEL_TOO_OLD
      AND NOT WIN32
      AND NOT CYGWIN
      AND NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES XL
      AND NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES PGI
      AND NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES Watcom)
    check_cxx_compiler_flag(-fvisibility=hidden COMPILER_HAS_HIDDEN_VISIBILITY)
    check_cxx_compiler_flag(-fvisibility-inlines-hidden
      COMPILER_HAS_HIDDEN_INLINE_VISIBILITY)
    option(USE_COMPILER_HIDDEN_VISIBILITY
      "Use HIDDEN visibility support if available." ON)
    mark_as_advanced(USE_COMPILER_HIDDEN_VISIBILITY)
  endif()
endmacro()

macro(_test_compiler_has_deprecated)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES Borland
      OR "${CMAKE_CXX_COMPILER_ID}" MATCHES HP
      OR GCC_TOO_OLD
      OR "${CMAKE_CXX_COMPILER_ID}" MATCHES PGI
      OR "${CMAKE_CXX_COMPILER_ID}" MATCHES Watcom)
    set(COMPILER_HAS_DEPRECATED "" CACHE INTERNAL
      "Compiler support for a deprecated attribute")
  else()
    _check_cxx_compiler_attribute("__attribute__((__deprecated__))"
      COMPILER_HAS_DEPRECATED_ATTR)
    if(COMPILER_HAS_DEPRECATED_ATTR)
      set(COMPILER_HAS_DEPRECATED "${COMPILER_HAS_DEPRECATED_ATTR}"
        CACHE INTERNAL "Compiler support for a deprecated attribute")
    else()
      _check_cxx_compiler_attribute("__declspec(deprecated)"
        COMPILER_HAS_DEPRECATED)
    endif()
  endif()
endmacro()

get_filename_component(_GENERATE_EXPORT_HEADER_MODULE_DIR
  "${CMAKE_CURRENT_LIST_FILE}" PATH)

macro(_DO_SET_MACRO_VALUES TARGET_LIBRARY)
  set(DEFINE_DEPRECATED)
  set(DEFINE_EXPORT)
  set(DEFINE_IMPORT)
  set(DEFINE_NO_EXPORT)

  if (COMPILER_HAS_DEPRECATED_ATTR)
    set(DEFINE_DEPRECATED "__attribute__ ((__deprecated__))")
  elseif(COMPILER_HAS_DEPRECATED)
    set(DEFINE_DEPRECATED "__declspec(deprecated)")
  endif()

  get_property(type TARGET ${TARGET_LIBRARY} PROPERTY TYPE)

  if(NOT ${type} STREQUAL "STATIC_LIBRARY")
    if(WIN32)
      set(DEFINE_EXPORT "__declspec(dllexport)")
      set(DEFINE_IMPORT "__declspec(dllimport)")
    elseif(COMPILER_HAS_HIDDEN_VISIBILITY AND USE_COMPILER_HIDDEN_VISIBILITY)
      set(DEFINE_EXPORT "__attribute__((visibility(\"default\")))")
      set(DEFINE_IMPORT "__attribute__((visibility(\"default\")))")
      set(DEFINE_NO_EXPORT "__attribute__((visibility(\"hidden\")))")
    endif()
  endif()
endmacro()

macro(_DO_GENERATE_EXPORT_HEADER TARGET_LIBRARY)
  # Option overrides
  set(options DEFINE_NO_DEPRECATED)
  set(oneValueArgs PREFIX_NAME BASE_NAME EXPORT_MACRO_NAME EXPORT_FILE_NAME
    DEPRECATED_MACRO_NAME NO_EXPORT_MACRO_NAME STATIC_DEFINE
    NO_DEPRECATED_MACRO_NAME)
  set(multiValueArgs)

  cmake_parse_arguments(_GEH "${options}" "${oneValueArgs}" "${multiValueArgs}"
    ${ARGN})

  set(BASE_NAME "${TARGET_LIBRARY}")

  if(_GEH_BASE_NAME)
    set(BASE_NAME ${_GEH_BASE_NAME})
  endif()

  string(TOUPPER ${BASE_NAME} BASE_NAME_UPPER)
  string(TOLOWER ${BASE_NAME} BASE_NAME_LOWER)

  # Default options
  set(EXPORT_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_EXPORT")
  set(NO_EXPORT_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_NO_EXPORT")
  set(EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME_LOWER}_export.h")
  set(DEPRECATED_MACRO_NAME "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_DEPRECATED")
  set(STATIC_DEFINE "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_STATIC_DEFINE")
  set(NO_DEPRECATED_MACRO_NAME
    "${_GEH_PREFIX_NAME}${BASE_NAME_UPPER}_NO_DEPRECATED")

  if(_GEH_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to GENERATE_EXPORT_HEADER(): \"${_GEH_UNPARSED_ARGUMENTS}\"")
  endif()

  if(_GEH_EXPORT_MACRO_NAME)
    set(EXPORT_MACRO_NAME ${_GEH_PREFIX_NAME}${_GEH_EXPORT_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${EXPORT_MACRO_NAME} EXPORT_MACRO_NAME)
  if(_GEH_EXPORT_FILE_NAME)
    if(IS_ABSOLUTE ${_GEH_EXPORT_FILE_NAME})
      set(EXPORT_FILE_NAME ${_GEH_EXPORT_FILE_NAME})
    else()
      set(EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${_GEH_EXPORT_FILE_NAME}")
    endif()
  endif()
  if(_GEH_DEPRECATED_MACRO_NAME)
    set(DEPRECATED_MACRO_NAME ${_GEH_PREFIX_NAME}${_GEH_DEPRECATED_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${DEPRECATED_MACRO_NAME} DEPRECATED_MACRO_NAME)
  if(_GEH_NO_EXPORT_MACRO_NAME)
    set(NO_EXPORT_MACRO_NAME ${_GEH_PREFIX_NAME}${_GEH_NO_EXPORT_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${NO_EXPORT_MACRO_NAME} NO_EXPORT_MACRO_NAME)
  if(_GEH_STATIC_DEFINE)
    set(STATIC_DEFINE ${_GEH_PREFIX_NAME}${_GEH_STATIC_DEFINE})
  endif()
  string(MAKE_C_IDENTIFIER ${STATIC_DEFINE} STATIC_DEFINE)

  if(_GEH_DEFINE_NO_DEPRECATED)
    set(DEFINE_NO_DEPRECATED TRUE)
  endif()

  if(_GEH_NO_DEPRECATED_MACRO_NAME)
    set(NO_DEPRECATED_MACRO_NAME
      ${_GEH_PREFIX_NAME}${_GEH_NO_DEPRECATED_MACRO_NAME})
  endif()
  string(MAKE_C_IDENTIFIER ${NO_DEPRECATED_MACRO_NAME} NO_DEPRECATED_MACRO_NAME)

  set(INCLUDE_GUARD_NAME "${EXPORT_MACRO_NAME}_H")

  get_target_property(EXPORT_IMPORT_CONDITION ${TARGET_LIBRARY} DEFINE_SYMBOL)

  if(NOT EXPORT_IMPORT_CONDITION)
    set(EXPORT_IMPORT_CONDITION ${TARGET_LIBRARY}_EXPORTS)
  endif()
  string(MAKE_C_IDENTIFIER ${EXPORT_IMPORT_CONDITION} EXPORT_IMPORT_CONDITION)

  configure_file("${_GENERATE_EXPORT_HEADER_MODULE_DIR}/exportheader.cmake.in"
    "${EXPORT_FILE_NAME}" @ONLY)
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
  _test_compiler_has_deprecated()
  _do_set_macro_values(${TARGET_LIBRARY})
  _do_generate_export_header(${TARGET_LIBRARY} ${ARGN})
endfunction()

function(add_compiler_export_flags)

  _test_compiler_hidden_visibility()
  _test_compiler_has_deprecated()

  if(NOT (USE_COMPILER_HIDDEN_VISIBILITY AND COMPILER_HAS_HIDDEN_VISIBILITY))
    # Just return if there are no flags to add.
    return()
  endif()

  set (EXTRA_FLAGS "-fvisibility=hidden")

  if(COMPILER_HAS_HIDDEN_INLINE_VISIBILITY)
    set (EXTRA_FLAGS "${EXTRA_FLAGS} -fvisibility-inlines-hidden")
  endif()

  # Either return the extra flags needed in the supplied argument, or to the
  # CMAKE_CXX_FLAGS if no argument is supplied.
  if(ARGV0)
    set(${ARGV0} "${EXTRA_FLAGS}" PARENT_SCOPE)
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_FLAGS}" PARENT_SCOPE)
  endif()
endfunction()
