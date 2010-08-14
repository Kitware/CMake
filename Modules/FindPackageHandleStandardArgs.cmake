# FIND_PACKAGE_HANDLE_STANDARD_ARGS(<name> ... )
#
# This function is intended to be used in FindXXX.cmake modules files.
# It handles the REQUIRED, QUIET and version-related arguments to FIND_PACKAGE().
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
#                                           [FAIL_MESSAGE "Custom failure message"] )
#
# As above, if <var1> through <varN> are all valid, <UPPERCASED_NAME>_FOUND
# will be set to TRUE.
# Via FAIL_MESSAGE a custom failure message can be specified, if this is not
# used, the default message will be displayed.
# Following VERSION_VAR the name of the variable can be specified which holds
# the version of the package which has been found. If this is done, this version
# will be checked against the (potentially) specified required version used
# in the find_package() call. The EXACT keyword is also handled. The default
# messages include information about the required version and the version
# which has been actually found, both if the version is ok or not.
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

INCLUDE(FindPackageMessage)
INCLUDE(CMakeParseArguments)


FUNCTION(FIND_PACKAGE_HANDLE_STANDARD_ARGS _NAME _FIRST_ARG _VAR1)

# set up the arguments for CMAKE_PARSE_ARGUMENTS and check whether we are in
# new extended or in the "old" mode:
  SET(options) # none
  SET(oneValueArgs FAIL_MESSAGE VERSION_VAR)
  SET(multiValueArgs REQUIRED_VARS)
  SET(_KEYWORDS_FOR_EXTENDED_MODE  ${options} ${oneValueArgs} ${multiValueArgs} )
  LIST(FIND _KEYWORDS_FOR_EXTENDED_MODE "${_FIRST_ARG}" INDEX)

  IF(${INDEX} EQUAL -1)
    SET(FPHSA_FAIL_MESSAGE ${_FIRST_ARG})
    SET(FPHSA_REQUIRED_VARS ${_VAR1} ${ARGN})
    SET(FPHSA_VERSION_VAR)
  ELSE(${INDEX} EQUAL -1)

    CMAKE_PARSE_ARGUMENTS(FPHSA "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${_FIRST_ARG} ${_VAR1} ${ARGN})

    IF(FPHSA_UNPARSED_ARGUMENTS)
      MESSAGE(FATAL_ERROR "Unknown keywords given to FIND_PACKAGE_HANDLE_STANDARD_ARGS(): \"${FPHSA_UNPARSED_ARGUMENTS}\"")
    ENDIF(FPHSA_UNPARSED_ARGUMENTS)

    IF(NOT FPHSA_FAIL_MESSAGE)
      SET(FPHSA_FAIL_MESSAGE  "DEFAULT_MSG")
    ENDIF(NOT FPHSA_FAIL_MESSAGE)
  ENDIF(${INDEX} EQUAL -1)

# now that we collected all arguments, process them

  IF("${FPHSA_FAIL_MESSAGE}" STREQUAL "DEFAULT_MSG")
    SET(FPHSA_FAIL_MESSAGE "Could NOT find ${_NAME}")
  ENDIF("${FPHSA_FAIL_MESSAGE}" STREQUAL "DEFAULT_MSG")

  IF(NOT FPHSA_REQUIRED_VARS)
    MESSAGE(FATAL_ERROR "No REQUIRED_VARS specified for FIND_PACKAGE_HANDLE_STANDARD_ARGS()")
  ENDIF(NOT FPHSA_REQUIRED_VARS)

  LIST(GET FPHSA_REQUIRED_VARS 0 _FIRST_REQUIRED_VAR)

  STRING(TOUPPER ${_NAME} _NAME_UPPER)

  # collect all variables which were not found, so they can be printed, so the 
  # user knows better what went wrong (#6375)
  SET(MISSING_VARS "")
  SET(DETAILS "")
  SET(${_NAME_UPPER}_FOUND TRUE)
  # check if all passed variables are valid
  FOREACH(_CURRENT_VAR ${FPHSA_REQUIRED_VARS})
    IF(NOT ${_CURRENT_VAR})
      SET(${_NAME_UPPER}_FOUND FALSE)
      SET(MISSING_VARS "${MISSING_VARS} ${_CURRENT_VAR}")
    ELSE(NOT ${_CURRENT_VAR})
      SET(DETAILS "${DETAILS}[${${_CURRENT_VAR}}]")
    ENDIF(NOT ${_CURRENT_VAR})
  ENDFOREACH(_CURRENT_VAR)


  # version handling:
  SET(VERSION_MSG "")
  SET(VERSION_OK TRUE)
  IF (${_NAME}_FIND_VERSION)

    # if the package was found, check for the version using <NAME>_FIND_VERSION
    IF (${_NAME_UPPER}_FOUND)
      SET(VERSION ${${FPHSA_VERSION_VAR}} )

      IF(VERSION)

        IF(${_NAME}_FIND_VERSION_EXACT)       # exact version required
          IF (NOT "${${_NAME}_FIND_VERSION}" VERSION_EQUAL "${VERSION}")
            SET(VERSION_MSG " Found version \"${VERSION}\", but required is exact version \"${${_NAME}_FIND_VERSION}\"")
            SET(VERSION_OK FALSE)
          ELSE (NOT "${${_NAME}_FIND_VERSION}" VERSION_EQUAL "${VERSION}")
            SET(VERSION_MSG " (found exact version \"${VERSION}\")")
          ENDIF (NOT "${${_NAME}_FIND_VERSION}" VERSION_EQUAL "${VERSION}")

        ELSE(${_NAME}_FIND_VERSION_EXACT)     # minimum version specified:
          IF ("${${_NAME}_FIND_VERSION}" VERSION_GREATER "${VERSION}")
            SET(VERSION_MSG " Found version \"${VERSION}\", but required is at least \"${${_NAME}_FIND_VERSION}\"")
            SET(VERSION_OK FALSE)
          ELSE ("${${_NAME}_FIND_VERSION}" VERSION_GREATER "${VERSION}")
            SET(VERSION_MSG " (found version \"${VERSION}\", required is \"${${_NAME}_FIND_VERSION}\")")
          ENDIF ("${${_NAME}_FIND_VERSION}" VERSION_GREATER "${VERSION}")
        ENDIF(${_NAME}_FIND_VERSION_EXACT)

# Uncomment the following two lines to see to which Find-modules the VERSION_VAR keywords still need to be added:
#      ELSE(VERSION)
#        SET(VERSION_MSG " (WARNING: Required version is \"${${_NAME}_FIND_VERSION}\", but version of ${_NAME} is unknown)")
      ENDIF(VERSION)

    # if the package was not found, but a version was given, add that to the output:
    ELSE (${_NAME_UPPER}_FOUND)
      IF(${_NAME}_FIND_VERSION_EXACT)
         SET(VERSION_MSG " (Required is exact version \"${${_NAME}_FIND_VERSION}\")")
      ELSE(${_NAME}_FIND_VERSION_EXACT)
         SET(VERSION_MSG " (Required is at least version \"${${_NAME}_FIND_VERSION}\")")
      ENDIF(${_NAME}_FIND_VERSION_EXACT)
    ENDIF (${_NAME_UPPER}_FOUND)
  ENDIF (${_NAME}_FIND_VERSION)

  IF(VERSION_OK)
    SET(DETAILS "${DETAILS}[v${VERSION}]")
  ELSE(VERSION_OK)
    SET(${_NAME_UPPER}_FOUND FALSE)
  ENDIF(VERSION_OK)


  # print the result:
  IF (${_NAME_UPPER}_FOUND)
    FIND_PACKAGE_MESSAGE(${_NAME} "Found ${_NAME}: ${${_FIRST_REQUIRED_VAR}} ${VERSION_MSG}" "${DETAILS}")
  ELSE (${_NAME_UPPER}_FOUND)
    IF(NOT VERSION_OK)

      IF (${_NAME}_FIND_REQUIRED)
          MESSAGE(FATAL_ERROR "${FPHSA_FAIL_MESSAGE}: ${VERSION_MSG} (found ${${_FIRST_REQUIRED_VAR}})")
      ELSE (${_NAME}_FIND_REQUIRED)
        IF (NOT ${_NAME}_FIND_QUIETLY)
          MESSAGE(STATUS "${FPHSA_FAIL_MESSAGE}: ${VERSION_MSG} (found ${${_FIRST_REQUIRED_VAR}})")
        ENDIF (NOT ${_NAME}_FIND_QUIETLY)
      ENDIF (${_NAME}_FIND_REQUIRED)

    ELSE(NOT VERSION_OK)

      IF (${_NAME}_FIND_REQUIRED)
          MESSAGE(FATAL_ERROR "${FPHSA_FAIL_MESSAGE} (missing: ${MISSING_VARS}) ${VERSION_MSG}")
      ELSE (${_NAME}_FIND_REQUIRED)
        IF (NOT ${_NAME}_FIND_QUIETLY)
          MESSAGE(STATUS "${FPHSA_FAIL_MESSAGE}  (missing: ${MISSING_VARS}) ${VERSION_MSG}")
        ENDIF (NOT ${_NAME}_FIND_QUIETLY)
      ENDIF (${_NAME}_FIND_REQUIRED)
    ENDIF(NOT VERSION_OK)

  ENDIF (${_NAME_UPPER}_FOUND)

  SET(${_NAME_UPPER}_FOUND ${${_NAME_UPPER}_FOUND} PARENT_SCOPE)

ENDFUNCTION(FIND_PACKAGE_HANDLE_STANDARD_ARGS _FIRST_ARG)
