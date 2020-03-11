# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindRuby
--------

Find Ruby

This module finds if Ruby is installed and determines where the
include files and libraries are.  Ruby 1.8, 1.9, 2.0 and 2.1 are
supported.

The minimum required version of Ruby can be specified using the
standard syntax, e.g.  find_package(Ruby 1.8)

It also determines what the name of the library is.  This code sets
the following variables:

``Ruby_EXECUTABLE``
  full path to the ruby binary
``Ruby_INCLUDE_DIRS``
  include dirs to be used when using the ruby library
``Ruby_LIBARY``
  full path to the ruby library
``Ruby_VERSION``
  the version of ruby which was found, e.g. "1.8.7"
``Ruby_FOUND``
  set to true if ruby ws found successfully

Also:

``Ruby_INCLUDE_PATH``
  same as Ruby_INCLUDE_DIRS, only provided for compatibility reasons, don't use it
#]=======================================================================]

#   Ruby_ARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"archdir"@:>@)'`
#   Ruby_SITEARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitearchdir"@:>@)'`
#   Ruby_SITEDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitelibdir"@:>@)'`
#   Ruby_LIBDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"libdir"@:>@)'`
#   Ruby_LIBRUBYARG=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"LIBRUBYARG_SHARED"@:>@)'`

# uncomment the following line to get debug output for this file
# set(_Ruby_DEBUG_OUTPUT TRUE)

# Determine the list of possible names of the ruby executable depending
# on which version of ruby is required
set(_Ruby_POSSIBLE_EXECUTABLE_NAMES ruby)

# if 1.9 is required, don't look for ruby18 and ruby1.8, default to version 1.8
if(DEFINED Ruby_FIND_VERSION_MAJOR AND DEFINED Ruby_FIND_VERSION_MINOR)
  set(Ruby_FIND_VERSION_SHORT_NODOT "${Ruby_FIND_VERSION_MAJOR}${Ruby_FIND_VERSION_MINOR}")
  # we can't construct that if only major version is given
  set(_Ruby_POSSIBLE_EXECUTABLE_NAMES
    ruby${Ruby_FIND_VERSION_MAJOR}.${Ruby_FIND_VERSION_MINOR}
    ruby${Ruby_FIND_VERSION_MAJOR}${Ruby_FIND_VERSION_MINOR}
    ${_Ruby_POSSIBLE_EXECUTABLE_NAMES})
else()
  set(Ruby_FIND_VERSION_SHORT_NODOT "18")
endif()

if(NOT Ruby_FIND_VERSION_EXACT)
  list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby2.4 ruby24)
  list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby2.3 ruby23)
  list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby2.2 ruby22)
  list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby2.1 ruby21)
  list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby2.0 ruby20)
  list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby1.9 ruby19)

  # if we want a version below 1.9, also look for ruby 1.8
  if("${Ruby_FIND_VERSION_SHORT_NODOT}" VERSION_LESS "19")
    list(APPEND _Ruby_POSSIBLE_EXECUTABLE_NAMES ruby1.8 ruby18)
  endif()

  list(REMOVE_DUPLICATES _Ruby_POSSIBLE_EXECUTABLE_NAMES)
endif()

find_program(Ruby_EXECUTABLE NAMES ${_Ruby_POSSIBLE_EXECUTABLE_NAMES})

if(Ruby_EXECUTABLE  AND NOT  Ruby_VERSION_MAJOR)
  function(_RUBY_CONFIG_VAR RBVAR OUTVAR)
    execute_process(COMMAND ${Ruby_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['${RBVAR}']"
      RESULT_VARIABLE _Ruby_SUCCESS
      OUTPUT_VARIABLE _Ruby_OUTPUT
      ERROR_QUIET)
    if(_Ruby_SUCCESS OR _Ruby_OUTPUT STREQUAL "")
      execute_process(COMMAND ${Ruby_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['${RBVAR}']"
        RESULT_VARIABLE _Ruby_SUCCESS
        OUTPUT_VARIABLE _Ruby_OUTPUT
        ERROR_QUIET)
    endif()
    set(${OUTVAR} "${_Ruby_OUTPUT}" PARENT_SCOPE)
  endfunction()


  # query the ruby version
  _RUBY_CONFIG_VAR("MAJOR" Ruby_VERSION_MAJOR)
  _RUBY_CONFIG_VAR("MINOR" Ruby_VERSION_MINOR)
  _RUBY_CONFIG_VAR("TEENY" Ruby_VERSION_PATCH)

  # query the different directories
  _RUBY_CONFIG_VAR("archdir" Ruby_ARCH_DIR)
  _RUBY_CONFIG_VAR("arch" Ruby_ARCH)
  _RUBY_CONFIG_VAR("rubyhdrdir" Ruby_HDR_DIR)
  _RUBY_CONFIG_VAR("rubyarchhdrdir" Ruby_ARCHHDR_DIR)
  _RUBY_CONFIG_VAR("libdir" Ruby_POSSIBLE_LIB_DIR)
  _RUBY_CONFIG_VAR("rubylibdir" Ruby_RUBY_LIB_DIR)

  # site_ruby
  _RUBY_CONFIG_VAR("sitearchdir" Ruby_SITEARCH_DIR)
  _RUBY_CONFIG_VAR("sitelibdir" Ruby_SITELIB_DIR)

  # vendor_ruby available ?
  execute_process(COMMAND ${Ruby_EXECUTABLE} -r vendor-specific -e "print 'true'"
    OUTPUT_VARIABLE Ruby_HAS_VENDOR_RUBY  ERROR_QUIET)

  if(Ruby_HAS_VENDOR_RUBY)
    _RUBY_CONFIG_VAR("vendorlibdir" Ruby_VENDORLIB_DIR)
    _RUBY_CONFIG_VAR("vendorarchdir" Ruby_VENDORARCH_DIR)
  endif()

  # save the results in the cache so we don't have to run ruby the next time again
  set(Ruby_VERSION_MAJOR    ${Ruby_VERSION_MAJOR}    CACHE PATH "The Ruby major version" FORCE)
  set(Ruby_VERSION_MINOR    ${Ruby_VERSION_MINOR}    CACHE PATH "The Ruby minor version" FORCE)
  set(Ruby_VERSION_PATCH    ${Ruby_VERSION_PATCH}    CACHE PATH "The Ruby patch version" FORCE)
  set(Ruby_ARCH_DIR         ${Ruby_ARCH_DIR}         CACHE PATH "The Ruby arch dir" FORCE)
  set(Ruby_HDR_DIR          ${Ruby_HDR_DIR}          CACHE PATH "The Ruby header dir (1.9+)" FORCE)
  set(Ruby_ARCHHDR_DIR      ${Ruby_ARCHHDR_DIR}      CACHE PATH "The Ruby arch header dir (2.0+)" FORCE)
  set(Ruby_POSSIBLE_LIB_DIR ${Ruby_POSSIBLE_LIB_DIR} CACHE PATH "The Ruby lib dir" FORCE)
  set(Ruby_RUBY_LIB_DIR     ${Ruby_RUBY_LIB_DIR}     CACHE PATH "The Ruby ruby-lib dir" FORCE)
  set(Ruby_SITEARCH_DIR     ${Ruby_SITEARCH_DIR}     CACHE PATH "The Ruby site arch dir" FORCE)
  set(Ruby_SITELIB_DIR      ${Ruby_SITELIB_DIR}      CACHE PATH "The Ruby site lib dir" FORCE)
  set(Ruby_HAS_VENDOR_RUBY  ${Ruby_HAS_VENDOR_RUBY}  CACHE BOOL "Vendor Ruby is available" FORCE)
  set(Ruby_VENDORARCH_DIR   ${Ruby_VENDORARCH_DIR}   CACHE PATH "The Ruby vendor arch dir" FORCE)
  set(Ruby_VENDORLIB_DIR    ${Ruby_VENDORLIB_DIR}    CACHE PATH "The Ruby vendor lib dir" FORCE)

  mark_as_advanced(
    Ruby_ARCH_DIR
    Ruby_ARCH
    Ruby_HDR_DIR
    Ruby_ARCHHDR_DIR
    Ruby_POSSIBLE_LIB_DIR
    Ruby_RUBY_LIB_DIR
    Ruby_SITEARCH_DIR
    Ruby_SITELIB_DIR
    Ruby_HAS_VENDOR_RUBY
    Ruby_VENDORARCH_DIR
    Ruby_VENDORLIB_DIR
    Ruby_VERSION_MAJOR
    Ruby_VERSION_MINOR
    Ruby_VERSION_PATCH
    )
endif()

# In case Ruby_EXECUTABLE could not be executed (e.g. cross compiling)
# try to detect which version we found. This is not too good.
if(Ruby_EXECUTABLE AND NOT Ruby_VERSION_MAJOR)
  # by default assume 1.8.0
  set(Ruby_VERSION_MAJOR 1)
  set(Ruby_VERSION_MINOR 8)
  set(Ruby_VERSION_PATCH 0)
  # check whether we found 1.9.x
  if(${Ruby_EXECUTABLE} MATCHES "ruby1\\.?9")
    set(Ruby_VERSION_MAJOR 1)
    set(Ruby_VERSION_MINOR 9)
  endif()
  # check whether we found 2.0.x
  if(${Ruby_EXECUTABLE} MATCHES "ruby2\\.?0")
    set(Ruby_VERSION_MAJOR 2)
    set(Ruby_VERSION_MINOR 0)
  endif()
  # check whether we found 2.1.x
  if(${Ruby_EXECUTABLE} MATCHES "ruby2\\.?1")
    set(Ruby_VERSION_MAJOR 2)
    set(Ruby_VERSION_MINOR 1)
  endif()
  # check whether we found 2.2.x
  if(${Ruby_EXECUTABLE} MATCHES "ruby2\\.?2")
    set(Ruby_VERSION_MAJOR 2)
    set(Ruby_VERSION_MINOR 2)
  endif()
  # check whether we found 2.3.x
  if(${Ruby_EXECUTABLE} MATCHES "ruby2\\.?3")
    set(Ruby_VERSION_MAJOR 2)
    set(Ruby_VERSION_MINOR 3)
  endif()
  # check whether we found 2.4.x
  if(${Ruby_EXECUTABLE} MATCHES "ruby2\\.?4")
    set(Ruby_VERSION_MAJOR 2)
    set(Ruby_VERSION_MINOR 4)
  endif()
endif()

if(Ruby_VERSION_MAJOR)
  set(Ruby_VERSION "${Ruby_VERSION_MAJOR}.${Ruby_VERSION_MINOR}.${Ruby_VERSION_PATCH}")
  set(_Ruby_VERSION_SHORT "${Ruby_VERSION_MAJOR}.${Ruby_VERSION_MINOR}")
  set(_Ruby_VERSION_SHORT_NODOT "${Ruby_VERSION_MAJOR}${Ruby_VERSION_MINOR}")
  set(_Ruby_NODOT_VERSION "${Ruby_VERSION_MAJOR}${Ruby_VERSION_MINOR}${Ruby_VERSION_PATCH}")
endif()

find_path(Ruby_INCLUDE_DIR
  NAMES ruby.h
  HINTS
    ${Ruby_HDR_DIR}
    ${Ruby_ARCH_DIR}
    /usr/lib/ruby/${_Ruby_VERSION_SHORT}/i586-linux-gnu/
)

set(Ruby_INCLUDE_DIRS ${Ruby_INCLUDE_DIR} )

# if ruby > 1.8 is required or if ruby > 1.8 was found, search for the config.h dir
if( "${Ruby_FIND_VERSION_SHORT_NODOT}" GREATER 18  OR  "${_Ruby_VERSION_SHORT_NODOT}" GREATER 18  OR  Ruby_HDR_DIR)
  find_path(Ruby_CONFIG_INCLUDE_DIR
    NAMES ruby/config.h  config.h
    HINTS
      ${Ruby_HDR_DIR}/${Ruby_ARCH}
      ${Ruby_ARCH_DIR}
      ${Ruby_ARCHHDR_DIR}
  )

  set(Ruby_INCLUDE_DIRS ${Ruby_INCLUDE_DIRS} ${Ruby_CONFIG_INCLUDE_DIR} )
endif()


# Determine the list of possible names for the ruby library
set(_Ruby_POSSIBLE_LIB_NAMES ruby ruby-static ruby${_Ruby_VERSION_SHORT} ruby${_Ruby_VERSION_SHORT_NODOT} ruby-${_Ruby_VERSION_SHORT} ruby-${Ruby_VERSION})

if(WIN32)
  set( _Ruby_MSVC_RUNTIME "" )
  if( MSVC_VERSION EQUAL 1200 )
    set( _Ruby_MSVC_RUNTIME "60" )
  endif()
  if( MSVC_VERSION EQUAL 1300 )
    set( _Ruby_MSVC_RUNTIME "70" )
  endif()
  if( MSVC_VERSION EQUAL 1310 )
    set( _Ruby_MSVC_RUNTIME "71" )
  endif()
  if( MSVC_VERSION EQUAL 1400 )
    set( _Ruby_MSVC_RUNTIME "80" )
  endif()
  if( MSVC_VERSION EQUAL 1500 )
    set( _Ruby_MSVC_RUNTIME "90" )
  endif()

  set(_Ruby_ARCH_PREFIX "")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_Ruby_ARCH_PREFIX "x64-")
  endif()

  list(APPEND _Ruby_POSSIBLE_LIB_NAMES
             "${_Ruby_ARCH_PREFIX}msvcr${_Ruby_MSVC_RUNTIME}-ruby${_Ruby_NODOT_VERSION}"
             "${_Ruby_ARCH_PREFIX}msvcr${_Ruby_MSVC_RUNTIME}-ruby${_Ruby_NODOT_VERSION}-static"
             "${_Ruby_ARCH_PREFIX}msvcrt-ruby${_Ruby_NODOT_VERSION}"
             "${_Ruby_ARCH_PREFIX}msvcrt-ruby${_Ruby_NODOT_VERSION}-static" )
endif()

find_library(Ruby_LIBARY NAMES ${_Ruby_POSSIBLE_LIB_NAMES} HINTS ${Ruby_POSSIBLE_LIB_DIR} )

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
set(_Ruby_REQUIRED_VARS Ruby_EXECUTABLE Ruby_INCLUDE_DIR Ruby_LIBARY)
if(_Ruby_VERSION_SHORT_NODOT GREATER 18)
  list(APPEND _Ruby_REQUIRED_VARS Ruby_CONFIG_INCLUDE_DIR)
endif()

if(_Ruby_DEBUG_OUTPUT)
  message(STATUS "--------FindRuby.cmake debug------------")
  message(STATUS "_Ruby_POSSIBLE_EXECUTABLE_NAMES: ${_Ruby_POSSIBLE_EXECUTABLE_NAMES}")
  message(STATUS "_Ruby_POSSIBLE_LIB_NAMES: ${_Ruby_POSSIBLE_LIB_NAMES}")
  message(STATUS "Ruby_ARCH_DIR: ${Ruby_ARCH_DIR}")
  message(STATUS "Ruby_HDR_DIR: ${Ruby_HDR_DIR}")
  message(STATUS "Ruby_POSSIBLE_LIB_DIR: ${Ruby_POSSIBLE_LIB_DIR}")
  message(STATUS "Found Ruby_VERSION: \"${Ruby_VERSION}\" , short: \"${_Ruby_VERSION_SHORT}\", nodot: \"${_Ruby_VERSION_SHORT_NODOT}\"")
  message(STATUS "_Ruby_REQUIRED_VARS: ${_Ruby_REQUIRED_VARS}")
  message(STATUS "Ruby_EXECUTABLE: ${Ruby_EXECUTABLE}")
  message(STATUS "Ruby_LIBARY: ${Ruby_LIBARY}")
  message(STATUS "Ruby_INCLUDE_DIR: ${Ruby_INCLUDE_DIR}")
  message(STATUS "Ruby_CONFIG_INCLUDE_DIR: ${Ruby_CONFIG_INCLUDE_DIR}")
  message(STATUS "--------------------")
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Ruby  REQUIRED_VARS  ${_Ruby_REQUIRED_VARS}
                                        VERSION_VAR Ruby_VERSION )

mark_as_advanced(
  Ruby_EXECUTABLE
  Ruby_LIBARY
  Ruby_INCLUDE_DIR
  Ruby_CONFIG_INCLUDE_DIR
  )

# Set some variables for compatibility with previous version of this file (no need to provide a CamelCase version of that...)
set(RUBY_POSSIBLE_LIB_PATH ${Ruby_POSSIBLE_LIB_DIR})
set(RUBY_RUBY_LIB_PATH ${Ruby_RUBY_LIB_DIR})
set(RUBY_INCLUDE_PATH ${Ruby_INCLUDE_DIRS})

# Backwards compatibility
# Define upper case versions of output variables
foreach(Camel
    Ruby_EXECUTABLE
    Ruby_INCLUDE_DIRS
    Ruby_LIBRARY
    Ruby_VERSION
    Ruby_VERSION_MAJOR
    Ruby_VERSION_MINOR
    Ruby_VERSION_PATCH
    Ruby_INCLUDE_PATH

    Ruby_ARCH_DIR
    Ruby_ARCH
    Ruby_HDR_DIR
    Ruby_ARCHHDR_DIR
    Ruby_POSSIBLE_LIB_DIR
    Ruby_RUBY_LIB_DIR
    Ruby_SITEARCH_DIR
    Ruby_SITELIB_DIR
    Ruby_HAS_VENDOR_RUBY
    Ruby_VENDORARCH_DIR

    )
    string(TOUPPER ${Camel} UPPER)
    set(${UPPER} ${${Camel}})
endforeach()
