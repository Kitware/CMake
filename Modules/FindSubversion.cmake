# - Extract information from a subversion working copy
# The module defines the following variables:
#  Subversion_SVN_EXECUTABLE - path to svn command line client
#  Subversion_VERSION_SVN - version of svn command line client
#  Subversion_FOUND - true if the command line client was found
#  SUBVERSION_FOUND - same as Subversion_FOUND, set for compatiblity reasons
#
# The minimum required version of Subversion can be specified using the
# standard syntax, e.g. FIND_PACKAGE(Subversion 1.4)
#
# If the command line client executable is found the following macros are
# defined:
#  Subversion_WC_INFO(<dir> <var-prefix>)
#  Subversion_WC_LOG(<dir> <var-prefix>)
#  Subversion_WC_UPDATE([path...])
# Subversion_WC_INFO extracts information of a subversion working copy at
# a given location. This macro defines the following variables:
#  <var-prefix>_WC_URL - url of the repository (at <dir>)
#  <var-prefix>_WC_ROOT - root url of the repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_INFO - output of command `svn info <dir>'
# Subversion_WC_LOG retrieves the log message of the base revision of a
# subversion working copy at a given location. This macro defines the
# variable:
#  <var-prefix>_LAST_CHANGED_LOG - last log of base revision
# Subversion_WC_UPDATE runs 'svn update' on each of the paths supplied as
# argument. Each path will be converted to an absolute. This is necessary in
# order to determine its position relative to the top-level source directory
# CMAKE_SOURCE_DIR.  If any intermediate directories are missing, they will be
# 'svn update'-d non-recursively as well.
#
# Example usage:
#  FIND_PACKAGE(Subversion)
#  IF(SUBVERSION_FOUND)
#    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Current revision is ${Project_WC_REVISION}")
#    Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Last changed log is ${Project_LAST_CHANGED_LOG}")
#    Subversion_WC_UPDATE(${PROJECT_SOURCE_DIR})
#    MESSAGE("Updated my working copy")
#    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    MESSAGE("Revision after update is ${Project_WC_REVISION}")
#  ENDIF(SUBVERSION_FOUND)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Tristan Carel
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

FIND_PROGRAM(Subversion_SVN_EXECUTABLE svn
  DOC "subversion command line client")
MARK_AS_ADVANCED(Subversion_SVN_EXECUTABLE)

IF(Subversion_SVN_EXECUTABLE)
  # the subversion commands should be executed with the C locale, otherwise
  # the message (which are parsed) may be translated, Alex
  SET(_Subversion_SAVED_LC_ALL "$ENV{LC_ALL}")
  SET(ENV{LC_ALL} C)

  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} --version
    OUTPUT_VARIABLE Subversion_VERSION_SVN
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # restore the previous LC_ALL
  SET(ENV{LC_ALL} ${_Subversion_SAVED_LC_ALL})

  STRING(REGEX REPLACE "^(.*\n)?svn, version ([.0-9]+).*"
    "\\2" Subversion_VERSION_SVN "${Subversion_VERSION_SVN}")

  MACRO(Subversion_WC_INFO dir prefix)
    # the subversion commands should be executed with the C locale, otherwise
    # the message (which are parsed) may be translated, Alex
    SET(_Subversion_SAVED_LC_ALL "$ENV{LC_ALL}")
    SET(ENV{LC_ALL} C)

    EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} info ${dir}
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE Subversion_svn_info_error
      RESULT_VARIABLE Subversion_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_svn_info_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${Subversion_SVN_EXECUTABLE} info ${dir}\" failed with output:\n${Subversion_svn_info_error}")
    ELSE(NOT ${Subversion_svn_info_result} EQUAL 0)

      STRING(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"
        "\\2" ${prefix}_WC_URL "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Repository Root: ([^\n]+).*"
        "\\2" ${prefix}_WC_ROOT "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
        "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_REV "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_DATE "${${prefix}_WC_INFO}")

    ENDIF(NOT ${Subversion_svn_info_result} EQUAL 0)

    # restore the previous LC_ALL
    SET(ENV{LC_ALL} ${_Subversion_SAVED_LC_ALL})

  ENDMACRO(Subversion_WC_INFO)

  MACRO(Subversion_WC_LOG dir prefix)
    # This macro can block if the certificate is not signed:
    # svn ask you to accept the certificate and wait for your answer
    # This macro requires a svn server network access (Internet most of the time)
    # and can also be slow since it access the svn server
    EXECUTE_PROCESS(COMMAND
      ${Subversion_SVN_EXECUTABLE} log -r BASE ${dir}
      OUTPUT_VARIABLE ${prefix}_LAST_CHANGED_LOG
      ERROR_VARIABLE Subversion_svn_log_error
      RESULT_VARIABLE Subversion_svn_log_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${Subversion_svn_log_result} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${Subversion_SVN_EXECUTABLE} log -r BASE ${dir}\" failed with output:\n${Subversion_svn_log_error}")
    ENDIF(NOT ${Subversion_svn_log_result} EQUAL 0)
  ENDMACRO(Subversion_WC_LOG)

  MACRO(Subversion_WC_UPDATE)
    # This macro requires network access to the svn server and could be slow.
    SET(_svn_update ${Subversion_SVN_EXECUTABLE} --non-interactive update -q)
    FOREACH(_path ${ARGV})
      IF(NOT IS_ABSOLUTE ${_path})
        GET_FILENAME_COMPONENT(_path ${_path} ABSOLUTE)
      ENDIF(NOT IS_ABSOLUTE ${_path})
      STRING(REGEX REPLACE "^${CMAKE_SOURCE_DIR}/" "" _dirs "${_path}")
      STRING(REPLACE "/" ";" _dirs "${_dirs}")
      SET(_wc ${CMAKE_SOURCE_DIR})
      FOREACH(_dir ${_dirs})
        SET(_wc ${_wc}/${_dir})
        IF("${_wc}" STREQUAL "${_path}")
          SET(_cmd ${_svn_update} "${_wc}")
        ELSE("${_wc}" STREQUAL "${_path}")
          SET(_cmd ${_svn_update} -N "${_wc}")
        ENDIF("${_wc}" STREQUAL "${_path}")
        EXECUTE_PROCESS(
          COMMAND ${_cmd}
          RESULT_VARIABLE _result
          ERROR_VARIABLE _error)
        IF(_result)
          MESSAGE(SEND_ERROR "${_cmd} failed:\n${_error}")
        ENDIF(_result)
      ENDFOREACH(_dir ${_dirs})
    ENDFOREACH(_path ${ARGV})
  ENDMACRO(Subversion_WC_UPDATE)

ENDIF(Subversion_SVN_EXECUTABLE)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Subversion REQUIRED_VARS Subversion_SVN_EXECUTABLE
                                             VERSION_VAR Subversion_VERSION_SVN )

# for compatibility
SET(Subversion_FOUND ${SUBVERSION_FOUND})
SET(Subversion_SVN_FOUND ${SUBVERSION_FOUND})
