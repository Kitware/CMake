# - Macros for generating a summary of enabled/disabled features
#
# PRINT_ENABLED_FEATURES()
#   Print a summary of all enabled features. By default all successfull
#   FIND_PACKAGE() calls will appear here, except the ones which used the
#   QUIET keyword. Additional features can be added by appending an entry
#   to the global ENABLED_FEATURES property. If SET_FEATURE_INFO() is
#   used for that feature, the output will be much more informative.
#
# PRINT_DISABLED_FEATURES()
#   Same as PRINT_ENABLED_FEATURES(), but for disabled features. It can
#   be extended the same way by adding to the global property
#   DISABLED_FEATURES.
#
# SET_FEATURE_INFO(NAME DESCRIPTION [URL [COMMENT] ] )
#    Use this macro to set up information about the named feature, which will
#    then be displayed by PRINT_ENABLED/DISABLED_FEATURES().
#    Example: SET_FEATURE_INFO(LibXml2 "XML processing library."
#    "http://xmlsoft.org/")
#

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

INCLUDE(CMakeParseArguments)

FUNCTION(SET_FEATURE_INFO _name _desc)
  SET(_url "${ARGV2}")
  SET(_comment "${ARGV3}")
  SET_PROPERTY(GLOBAL PROPERTY _CMAKE_${_name}_DESCRIPTION "${_desc}" )
  IF(_url MATCHES ".+")
    SET_PROPERTY(GLOBAL PROPERTY _CMAKE_${_name}_URL "${_url}" )
  ENDIF(_url MATCHES ".+")
  IF(_comment MATCHES ".+")
    SET_PROPERTY(GLOBAL PROPERTY _CMAKE_${_name}_COMMENT "${_comment}" )
  ENDIF(_comment MATCHES ".+")
ENDFUNCTION(SET_FEATURE_INFO)


FUNCTION(_FS_GET_FEATURE_SUMMARY _property _var)
  SET(_currentFeatureText "")
  GET_PROPERTY(_EnabledFeatures  GLOBAL PROPERTY ${_property})
  FOREACH(_currentFeature ${_EnabledFeatures})
    SET(_currentFeatureText "${_currentFeatureText}\n${_currentFeature}")
    GET_PROPERTY(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_REQUIRED_VERSION)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} (required version ${_info})")
    ENDIF(_info)
    GET_PROPERTY(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_DESCRIPTION)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} , ${_info}")
    ENDIF(_info)
    GET_PROPERTY(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_URL)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} , <${_info}>")
    ENDIF(_info)
    GET_PROPERTY(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_COMMENT)
    IF(_info)
      SET(_currentFeatureText "${_currentFeatureText} , ${_info}")
    ENDIF(_info)
  ENDFOREACH(_currentFeature)
  SET(${_var} "${_currentFeatureText}" PARENT_SCOPE)
ENDFUNCTION(_FS_GET_FEATURE_SUMMARY)


FUNCTION(PRINT_ENABLED_FEATURES)
  FEATURE_SUMMARY(WHAT ENABLED_FEATURES  DESCRIPTION "Enabled features:")
ENDFUNCTION(PRINT_ENABLED_FEATURES)


FUNCTION(PRINT_DISABLED_FEATURES)
  FEATURE_SUMMARY(WHAT DISABLED_FEATURES  DESCRIPTION "Disabled features:")
ENDFUNCTION(PRINT_DISABLED_FEATURES)

# feature_summary(FILENAME foo.log
#                 APPEND TRUE
#                 VAR <variable_name>
#                 DESCRIPTION "Found packages:"
#                 WHAT [ALL PACKAGES_FOUND PACKAGES_NOT_FOUND ENABLED_FEATURES DISABLED_FEATURES]
#                 )


FUNCTION(FEATURE_SUMMARY)
# CMAKE_PARSE_ARGUMENTS(<prefix> <options> <one_value_keywords> <multi_value_keywords> args...)
  SET(options APPEND)
  SET(oneValueArgs FILENAME VAR DESCRIPTION WHAT)
  SET(multiValueArgs ) # none

  CMAKE_PARSE_ARGUMENTS(_FS "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${_FIRST_ARG} ${ARGN})

  IF(_FS_UNPARSED_ARGUMENTS)
    MESSAGE(FATAL_ERROR "Unknown keywords given to FEATURE_SUMMARY(): \"${_FS_UNPARSED_ARGUMENTS}\"")
  ENDIF(_FS_UNPARSED_ARGUMENTS)

  IF(NOT _FS_WHAT)
    MESSAGE(FATAL_ERROR "The call to FEATURE_SUMMAY() doesn't set the required WHAT argument.")
  ENDIF(NOT _FS_WHAT)

  IF(   "${_FS_WHAT}" STREQUAL "ENABLED_FEATURES"
     OR "${_FS_WHAT}" STREQUAL "DISABLED_FEATURES"
     OR "${_FS_WHAT}" STREQUAL "PACKAGES_FOUND"
     OR "${_FS_WHAT}" STREQUAL "PACKAGES_NOT_FOUND")
    _FS_GET_FEATURE_SUMMARY( ${_FS_WHAT} _featureSummary)
  ELSEIF("${_FS_WHAT}" STREQUAL "ALL")
    _FS_GET_FEATURE_SUMMARY( PACKAGES_FOUND _tmp1)
    _FS_GET_FEATURE_SUMMARY( PACKAGES_NOT_FOUND _tmp2)
    SET(_featureSummary "${_tmp1}${_tmp2}")
  ELSE()
    MESSAGE(FATAL_ERROR "The WHAT argument of FEATURE_SUMMARY() is set to ${_FS_WHAT}, which is not a valid value.")
  ENDIF()

  SET(_fullText "${_FS_DESCRIPTION}${_featureSummary}\n")

  IF(_FS_FILENAME)
    IF(_FS_APPEND)
      FILE(WRITE  "${_FS_FILENAME}" "${_fullText}")
    ELSE(_FS_APPEND)
      FILE(APPEND "${_FS_FILENAME}" "${_fullText}")
    ENDIF(_FS_APPEND)

  ELSE(_FS_FILENAME)
    IF(NOT _FS_VAR)
      MESSAGE(STATUS "${_fullText}")
    ENDIF(NOT _FS_VAR)
  ENDIF(_FS_FILENAME)

  IF(_FS_VAR)
    SET(${_FS_VAR} "${_fullText}" PARENT_SCOPE)
  ENDIF(_FS_VAR)

ENDFUNCTION(FEATURE_SUMMARY)
