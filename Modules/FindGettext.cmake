# - Find GNU gettext tools
# This module looks for the GNU gettext tools. This module defines the
# following values:
#  GETTEXT_MSGMERGE_EXECUTABLE: the full path to the msgmerge tool.
#  GETTEXT_MSGFMT_EXECUTABLE: the full path to the msgfmt tool.
#  GETTEXT_FOUND: True if gettext has been found.
#  GETTEXT_VERSION_STRING: the version of gettext found (since CMake 2.8.8)
#
# Additionally it provides the following macros:
# GETTEXT_CREATE_TRANSLATIONS ( outputFile [ALL] file1 ... fileN )
#    This will create a target "translations" which will convert the
#    given input po files into the binary output mo file. If the
#    ALL option is used, the translations will also be created when
#    building the default target.
# GETTEXT_PROCESS_POT( <potfile> [ALL] [INSTALL_DESTINATION <destdir>] LANGUAGES <lang1> <lang2> ... )
#     Process the given pot file to mo files.
#     If INSTALL_DESTINATION is given then automatically install rules will be created,
#     the language subdirectory will be taken into account (by default use share/locale/).
#     If ALL is specified, the pot file is processed when building the all traget.
#     It creates a custom target "potfile".
# GETTEXT_PROCESS_PO_FILES( <lang> [ALL] [INSTALL_DESTINATION <dir>] PO_FILES <po1> <po2> ... )
#     Process the given po files to mo files for the given language.
#     If INSTALL_DESTINATION is given then automatically install rules will be created,
#     the language subdirectory will be taken into account (by default use share/locale/).
#     If ALL is specified, the po files are processed when building the all traget.
#     It creates a custom target "pofiles".

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
# Copyright 2007      Alexander Neundorf <neundorf@kde.org>
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

FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)

FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(GETTEXT_MSGMERGE_EXECUTABLE)
   EXECUTE_PROCESS(COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --version
                  OUTPUT_VARIABLE gettext_version
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
   IF (gettext_version MATCHES "^msgmerge \\(.*\\) [0-9]")
      STRING(REGEX REPLACE "^msgmerge \\([^\\)]*\\) ([0-9\\.]+[^ \n]*).*" "\\1" GETTEXT_VERSION_STRING "${gettext_version}")
   ENDIF()
   UNSET(gettext_version)
ENDIF(GETTEXT_MSGMERGE_EXECUTABLE)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gettext
                                  REQUIRED_VARS GETTEXT_MSGMERGE_EXECUTABLE GETTEXT_MSGFMT_EXECUTABLE
                                  VERSION_VAR GETTEXT_VERSION_STRING)

INCLUDE(CMakeParseArguments)

FUNCTION(_GETTEXT_GET_UNIQUE_TARGET_NAME _name _unique_name)
   SET(propertyName "_GETTEXT_UNIQUE_COUNTER_${_name}")
   GET_PROPERTY(currentCounter GLOBAL PROPERTY "${propertyName}")
   IF(NOT currentCounter)
      SET(currentCounter 1)
   ENDIF()
   SET(${_unique_name} "${_name}_${currentCounter}" PARENT_SCOPE)
   MATH(EXPR currentCounter "${currentCounter} + 1")
   SET_PROPERTY(GLOBAL PROPERTY ${propertyName} ${currentCounter} )
ENDFUNCTION()

MACRO(GETTEXT_CREATE_TRANSLATIONS _potFile _firstPoFileArg)
   # make it a real variable, so we can modify it here
   SET(_firstPoFile "${_firstPoFileArg}")

   SET(_gmoFiles)
   GET_FILENAME_COMPONENT(_potName ${_potFile} NAME)
   STRING(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _potBasename ${_potName})
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

   SET(_addToAll)
   IF(${_firstPoFile} STREQUAL "ALL")
      SET(_addToAll "ALL")
      SET(_firstPoFile)
   ENDIF(${_firstPoFile} STREQUAL "ALL")

   FOREACH (_currentPoFile ${_firstPoFile} ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
      GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_gmoFile}
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s ${_absFile} ${_absPotFile}
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absPotFile} ${_absFile}
      )

      INSTALL(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})

   ENDFOREACH (_currentPoFile )

   IF(NOT TARGET translations)
      ADD_CUSTOM_TARGET(translations)
   ENDIF()

  _GETTEXT_GET_UNIQUE_TARGET_NAME(translations uniqueTargetName)

   ADD_CUSTOM_TARGET(${uniqueTargetName} ${_addToAll} DEPENDS ${_gmoFiles})

   ADD_DEPENDENCIES(translations ${uniqueTargetName})

ENDMACRO(GETTEXT_CREATE_TRANSLATIONS )


FUNCTION(GETTEXT_PROCESS_POT_FILE _potFile)
   SET(_gmoFiles)
   SET(_options ALL)
   SET(_oneValueArgs INSTALL_DESTINATION)
   SET(_multiValueArgs LANGUAGES)

   CMAKE_PARSE_ARGUMENTS(_parsedArguments "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

   GET_FILENAME_COMPONENT(_potName ${_potFile} NAME)
   STRING(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _potBasename ${_potName})
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

   FOREACH (_lang ${_parsedArguments_LANGUAGES})
      SET(_poFile  "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.po")
      SET(_gmoFile "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo")

      ADD_CUSTOM_COMMAND(
         OUTPUT "${_poFile}"
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --update --backup=none -s ${_poFile} ${_absPotFile}
         DEPENDS ${_absPotFile}
      )

      ADD_CUSTOM_COMMAND(
         OUTPUT "${_gmoFile}"
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_poFile}
         DEPENDS ${_absPotFile} ${_poFile}
      )

      IF(_parsedArguments_INSTALL_DESTINATION)
         INSTALL(FILES ${_gmoFile} DESTINATION ${_parsedArguments_INSTALL_DESTINATION}/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
      ENDIF(_parsedArguments_INSTALL_DESTINATION)
      LIST(APPEND _gmoFiles ${_gmoFile})
   ENDFOREACH (_lang )

  IF(NOT TARGET potfiles)
     ADD_CUSTOM_TARGET(potfiles)
  ENDIF()

  _GETTEXT_GET_UNIQUE_TARGET_NAME( potfiles uniqueTargetName)

   IF(_parsedArguments_ALL)
      ADD_CUSTOM_TARGET(${uniqueTargetName} ALL DEPENDS ${_gmoFiles})
   ELSE(_parsedArguments_ALL)
      ADD_CUSTOM_TARGET(${uniqueTargetName} DEPENDS ${_gmoFiles})
   ENDIF(_parsedArguments_ALL)

   ADD_DEPENDENCIES(potfiles ${uniqueTargetName})

ENDFUNCTION(GETTEXT_PROCESS_POT_FILE)


FUNCTION(GETTEXT_PROCESS_PO_FILES _lang)
   SET(_options ALL)
   SET(_oneValueArgs INSTALL_DESTINATION)
   SET(_multiValueArgs PO_FILES)
   SET(_gmoFiles)

   CMAKE_PARSE_ARGUMENTS(_parsedArguments "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

   FOREACH(_current_PO_FILE ${_parsedArguments_PO_FILES})
      GET_FILENAME_COMPONENT(_name ${_current_PO_FILE} NAME)
      STRING(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _basename ${_name})
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.gmo)
      ADD_CUSTOM_COMMAND(OUTPUT ${_gmoFile}
            COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_current_PO_FILE}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS ${_current_PO_FILE}
         )

      IF(_parsedArguments_INSTALL_DESTINATION)
         INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.gmo DESTINATION ${_parsedArguments_INSTALL_DESTINATION}/${_lang}/LC_MESSAGES/ RENAME ${_basename}.mo)
      ENDIF(_parsedArguments_INSTALL_DESTINATION)
      LIST(APPEND _gmoFiles ${_gmoFile})
   ENDFOREACH(_current_PO_FILE)


  IF(NOT TARGET pofiles)
     ADD_CUSTOM_TARGET(pofiles)
  ENDIF()

  _GETTEXT_GET_UNIQUE_TARGET_NAME( pofiles uniqueTargetName)

   IF(_parsedArguments_ALL)
      ADD_CUSTOM_TARGET(${uniqueTargetName} ALL DEPENDS ${_gmoFiles})
   ELSE(_parsedArguments_ALL)
      ADD_CUSTOM_TARGET(${uniqueTargetName} DEPENDS ${_gmoFiles})
   ENDIF(_parsedArguments_ALL)

   ADD_DEPENDENCIES(pofiles ${uniqueTargetName})

ENDFUNCTION(GETTEXT_PROCESS_PO_FILES)

IF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND TRUE)
ELSE (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND FALSE)
   IF (GetText_REQUIRED)
      MESSAGE(FATAL_ERROR "GetText not found")
   ENDIF (GetText_REQUIRED)
ENDIF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
