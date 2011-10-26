# FIND_PACKAGE_HANDLE_STANDARD_ARGS(<name> ... )
#
# This function is intended to be used in FindXXX.cmake modules files.
# It handles the REQUIRED, QUIET and version-related arguments to find_package().
# It also sets the <UPPERCASED_NAME>_FOUND variable.
# The package is considered found if all variables <var1>... listed contain
# valid results, e.g. valid filepaths.
#
# There are two modes of this function. The first argument in both modes is
# the name of the Find-module where it is called (in original casing).
#
# The first simple mode looks like this:
#    FIND_PACKAGE_HANDLE_STANDARD_ARGS(<name> (DEFAULT_MSG|"Custom failure message") <var1>...<varN> )
# If the variables <var1> to <varN> are all valid, then <UPPERCASED_NAME>_FOUND
# will be set to TRUE.
# If DEFAULT_MSG is given as second argument, then the function will generate
# itself useful success and error messages. You can also supply a custom error message
# for the failure case. This is not recommended.
#
# The second mode is more powerful and also supports version checking:
#    FIND_PACKAGE_HANDLE_STANDARD_ARGS(NAME [REQUIRED_VARS <var1>...<varN>]
#                                           [VERSION_VAR   <versionvar>
#                                           [CONFIG_MODE]
#                                           [FAIL_MESSAGE "Custom failure message"] )
#
# As above, if <var1> through <varN> are all valid, <UPPERCASED_NAME>_FOUND
# will be set to TRUE.
# After REQUIRED_VARS the variables which are required for this package are listed.
# Following VERSION_VAR the name of the variable can be specified which holds
# the version of the package which has been found. If this is done, this version
# will be checked against the (potentially) specified required version used
# in the find_package() call. The EXACT keyword is also handled. The default
# messages include information about the required version and the version
# which has been actually found, both if the version is ok or not.
# Use the option CONFIG_MODE if your FindXXX.cmake module is a wrapper for
# a find_package(... NO_MODULE) call, in this case all the information
# provided by the config-mode of find_package() will be evaluated
# automatically.
# Via FAIL_MESSAGE a custom failure message can be specified, if this is not
# used, the default message will be displayed.
#
# Example for mode 1:
#
#    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXml2  DEFAULT_MSG  LIBXML2_LIBRARY LIBXML2_INCLUDE_DIR)
#
# LibXml2 is considered to be found, if both LIBXML2_LIBRARY and
# LIBXML2_INCLUDE_DIR are valid. Then also LIBXML2_FOUND is set to TRUE.
# If it is not found and REQUIRED was used, it fails with FATAL_ERROR,
# independent whether QUIET was used or not.
# If it is found, success will be reported, including the content of <var1>.
# On repeated Cmake runs, the same message won't be printed again.
#
# Example for mode 2:
#
#    FIND_PACKAGE_HANDLE_STANDARD_ARGS(BISON  REQUIRED_VARS BISON_EXECUTABLE
#                                             VERSION_VAR BISON_VERSION)
# In this case, BISON is considered to be found if the variable(s) listed
# after REQUIRED_VAR are all valid, i.e. BISON_EXECUTABLE in this case.
# Also the version of BISON will be checked by using the version contained
# in BISON_VERSION.
# Since no FAIL_MESSAGE is given, the default messages will be printed.
#
# Another example for mode 2:
#
#    find_package(Automoc4 QUIET NO_MODULE HINTS /opt/automoc4)
#    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Automoc4  CONFIG_MODE)
# In this case, FindAutmoc4.cmake wraps a call to find_package(Automoc4 NO_MODULE)
# and adds an additional search directory for automoc4.
# The following FIND_PACKAGE_HANDLE_STANDARD_ARGS() call produces a proper
# success/error message.

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

include(FindPackageMessage)
include(CMakeParseArguments)

# internal helper macro
macro(_FPHSA_FAILURE_MESSAGE _msg)
  if(${_NAME}_FIND_REQUIRED)
    message(FATAL_ERROR "${_msg}")
  else(${_NAME}_FIND_REQUIRED)
    if(NOT ${_NAME}_FIND_QUIETLY)
      message(STATUS "${_msg}")
    endif(NOT ${_NAME}_FIND_QUIETLY)
  endif(${_NAME}_FIND_REQUIRED)
endmacro(_FPHSA_FAILURE_MESSAGE _msg)


# internal helper macro to generate the failure message when used in CONFIG_MODE:
macro(_FPHSA_HANDLE_FAILURE_CONFIG_MODE)
  # <name>_CONFIG is set, but FOUND is false, this means that some other of the REQUIRED_VARS was not found:
  if(${_NAME}_CONFIG)
    _FPHSA_FAILURE_MESSAGE("${FPHSA_FAIL_MESSAGE}: missing: ${MISSING_VARS} (found ${${_NAME}_CONFIG} ${VERSION_MSG})")
  else(${_NAME}_CONFIG)
    # If _CONSIDERED_CONFIGS is set, the config-file has been found, but no suitable version.
    # List them all in the error message:
    if(${_NAME}_CONSIDERED_CONFIGS)
      set(configsText "")
      list(LENGTH ${_NAME}_CONSIDERED_CONFIGS configsCount)
      math(EXPR configsCount "${configsCount} - 1")
      foreach(currentConfigIndex RANGE ${configsCount})
        list(GET ${_NAME}_CONSIDERED_CONFIGS ${currentConfigIndex} filename)
        list(GET ${_NAME}_CONSIDERED_VERSIONS ${currentConfigIndex} version)
        set(configsText "${configsText}    ${filename} (version ${version})\n")
      endforeach(currentConfigIndex)
      _FPHSA_FAILURE_MESSAGE("${FPHSA_FAIL_MESSAGE} ${VERSION_MSG}, checked the following files:\n${configsText}")

    else(${_NAME}_CONSIDERED_CONFIGS)
      # Simple case: No Config-file was found at all:
      _FPHSA_FAILURE_MESSAGE("${FPHSA_FAIL_MESSAGE}: found neither ${_NAME}Config.cmake nor ${_NAME_LOWER}-config.cmake ${VERSION_MSG}")
    endif(${_NAME}_CONSIDERED_CONFIGS)
  endif(${_NAME}_CONFIG)
endmacro(_FPHSA_HANDLE_FAILURE_CONFIG_MODE)


function(FIND_PACKAGE_HANDLE_STANDARD_ARGS _NAME _FIRST_ARG)

# set up the arguments for CMAKE_PARSE_ARGUMENTS and check whether we are in
# new extended or in the "old" mode:
  set(options CONFIG_MODE)
  set(oneValueArgs FAIL_MESSAGE VERSION_VAR)
  set(multiValueArgs REQUIRED_VARS)
  set(_KEYWORDS_FOR_EXTENDED_MODE  ${options} ${oneValueArgs} ${multiValueArgs} )
  list(FIND _KEYWORDS_FOR_EXTENDED_MODE "${_FIRST_ARG}" INDEX)

  if(${INDEX} EQUAL -1)
    set(FPHSA_FAIL_MESSAGE ${_FIRST_ARG})
    set(FPHSA_REQUIRED_VARS ${ARGN})
    set(FPHSA_VERSION_VAR)
  else(${INDEX} EQUAL -1)

    CMAKE_PARSE_ARGUMENTS(FPHSA "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${_FIRST_ARG} ${ARGN})

    if(FPHSA_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Unknown keywords given to FIND_PACKAGE_HANDLE_STANDARD_ARGS(): \"${FPHSA_UNPARSED_ARGUMENTS}\"")
    endif(FPHSA_UNPARSED_ARGUMENTS)

    if(NOT FPHSA_FAIL_MESSAGE)
      set(FPHSA_FAIL_MESSAGE  "DEFAULT_MSG")
    endif(NOT FPHSA_FAIL_MESSAGE)
  endif(${INDEX} EQUAL -1)

# now that we collected all arguments, process them

  if("${FPHSA_FAIL_MESSAGE}" STREQUAL "DEFAULT_MSG")
    set(FPHSA_FAIL_MESSAGE "Could NOT find ${_NAME}")
  endif("${FPHSA_FAIL_MESSAGE}" STREQUAL "DEFAULT_MSG")

  # In config-mode, we rely on the variable <package>_CONFIG, which is set by find_package()
  # when it successfully found the config-file, including version checking:
  if(FPHSA_CONFIG_MODE)
    list(INSERT FPHSA_REQUIRED_VARS 0 ${_NAME}_CONFIG)
    list(REMOVE_DUPLICATES FPHSA_REQUIRED_VARS)
    set(FPHSA_VERSION_VAR ${_NAME}_VERSION)
  endif(FPHSA_CONFIG_MODE)

  if(NOT FPHSA_REQUIRED_VARS)
    message(FATAL_ERROR "No REQUIRED_VARS specified for FIND_PACKAGE_HANDLE_STANDARD_ARGS()")
  endif(NOT FPHSA_REQUIRED_VARS)

  list(GET FPHSA_REQUIRED_VARS 0 _FIRST_REQUIRED_VAR)

  string(TOUPPER ${_NAME} _NAME_UPPER)
  string(TOLOWER ${_NAME} _NAME_LOWER)

  # collect all variables which were not found, so they can be printed, so the
  # user knows better what went wrong (#6375)
  set(MISSING_VARS "")
  set(DETAILS "")
  set(${_NAME_UPPER}_FOUND TRUE)
  # check if all passed variables are valid
  foreach(_CURRENT_VAR ${FPHSA_REQUIRED_VARS})
    if(NOT ${_CURRENT_VAR})
      set(${_NAME_UPPER}_FOUND FALSE)
      set(MISSING_VARS "${MISSING_VARS} ${_CURRENT_VAR}")
    else(NOT ${_CURRENT_VAR})
      set(DETAILS "${DETAILS}[${${_CURRENT_VAR}}]")
    endif(NOT ${_CURRENT_VAR})
  endforeach(_CURRENT_VAR)


  # version handling:
  set(VERSION_MSG "")
  set(VERSION_OK TRUE)
  set(VERSION ${${FPHSA_VERSION_VAR}} )
  if(${_NAME}_FIND_VERSION)

    if(VERSION)

      if(${_NAME}_FIND_VERSION_EXACT)       # exact version required
        if(NOT "${${_NAME}_FIND_VERSION}" VERSION_EQUAL "${VERSION}")
          set(VERSION_MSG "Found unsuitable version \"${VERSION}\", but required is exact version \"${${_NAME}_FIND_VERSION}\"")
          set(VERSION_OK FALSE)
        else(NOT "${${_NAME}_FIND_VERSION}" VERSION_EQUAL "${VERSION}")
          set(VERSION_MSG "(found suitable exact version \"${VERSION}\")")
        endif(NOT "${${_NAME}_FIND_VERSION}" VERSION_EQUAL "${VERSION}")

      else(${_NAME}_FIND_VERSION_EXACT)     # minimum version specified:
        if("${${_NAME}_FIND_VERSION}" VERSION_GREATER "${VERSION}")
          set(VERSION_MSG "Found unsuitable version \"${VERSION}\", but required is at least \"${${_NAME}_FIND_VERSION}\"")
          set(VERSION_OK FALSE)
        else("${${_NAME}_FIND_VERSION}" VERSION_GREATER "${VERSION}")
          set(VERSION_MSG "(found suitable version \"${VERSION}\", required is \"${${_NAME}_FIND_VERSION}\")")
        endif("${${_NAME}_FIND_VERSION}" VERSION_GREATER "${VERSION}")
      endif(${_NAME}_FIND_VERSION_EXACT)

    else(VERSION)

      # if the package was not found, but a version was given, add that to the output:
      if(${_NAME}_FIND_VERSION_EXACT)
         set(VERSION_MSG "(Required is exact version \"${${_NAME}_FIND_VERSION}\")")
      else(${_NAME}_FIND_VERSION_EXACT)
         set(VERSION_MSG "(Required is at least version \"${${_NAME}_FIND_VERSION}\")")
      endif(${_NAME}_FIND_VERSION_EXACT)

    endif(VERSION)
  else(${_NAME}_FIND_VERSION)
    if(VERSION)
      set(VERSION_MSG "(found version \"${VERSION}\")")
    endif(VERSION)
  endif(${_NAME}_FIND_VERSION)

  if(VERSION_OK)
    set(DETAILS "${DETAILS}[v${VERSION}(${${_NAME}_FIND_VERSION})]")
  else(VERSION_OK)
    set(${_NAME_UPPER}_FOUND FALSE)
  endif(VERSION_OK)


  # print the result:
  if(${_NAME_UPPER}_FOUND)
    FIND_PACKAGE_MESSAGE(${_NAME} "Found ${_NAME}: ${${_FIRST_REQUIRED_VAR}} ${VERSION_MSG}" "${DETAILS}")
  else(${_NAME_UPPER}_FOUND)

    if(FPHSA_CONFIG_MODE)
      _FPHSA_HANDLE_FAILURE_CONFIG_MODE()
    else(FPHSA_CONFIG_MODE)
      if(NOT VERSION_OK)
        _FPHSA_FAILURE_MESSAGE("${FPHSA_FAIL_MESSAGE}: ${VERSION_MSG} (found ${${_FIRST_REQUIRED_VAR}})")
      else(NOT VERSION_OK)
        _FPHSA_FAILURE_MESSAGE("${FPHSA_FAIL_MESSAGE} (missing: ${MISSING_VARS}) ${VERSION_MSG}")
      endif(NOT VERSION_OK)
    endif(FPHSA_CONFIG_MODE)

  endif(${_NAME_UPPER}_FOUND)

  set(${_NAME_UPPER}_FOUND ${${_NAME_UPPER}_FOUND} PARENT_SCOPE)

endfunction(FIND_PACKAGE_HANDLE_STANDARD_ARGS _FIRST_ARG)
