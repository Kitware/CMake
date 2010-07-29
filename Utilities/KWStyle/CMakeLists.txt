#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

#-----------------------------------------------------------------------------
# CMake uses KWStyle for checking the coding style
OPTION(CMAKE_USE_KWSTYLE "Run KWStyle in order to check for violations of the coding standard." OFF)
MARK_AS_ADVANCED(CMAKE_USE_KWSTYLE)

IF(CMAKE_USE_KWSTYLE)
  FIND_PROGRAM(KWSTYLE_EXECUTABLE
  NAMES KWStyle 
  PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware Inc.\\KWStyle 1.0.0]/bin"
  )
  MARK_AS_ADVANCED(KWSTYLE_EXECUTABLE)

  OPTION(KWSTYLE_USE_VIM_FORMAT "Set KWStyle to generate errors with a VIM-compatible format." OFF)
  OPTION(KWSTYLE_USE_MSVC_FORMAT "Set KWStyle to generate errors with a VisualStudio-compatible format." OFF)
  MARK_AS_ADVANCED(KWSTYLE_USE_VIM_FORMAT)
  MARK_AS_ADVANCED(KWSTYLE_USE_MSVC_FORMAT)

  IF(KWSTYLE_USE_VIM_FORMAT)
    SET(KWSTYLE_ARGUMENTS -vim ${KWSTYLE_ARGUMENTS})
  ENDIF(KWSTYLE_USE_VIM_FORMAT)

  IF(KWSTYLE_USE_MSVC_FORMAT)
    SET(KWSTYLE_ARGUMENTS -msvc ${KWSTYLE_ARGUMENTS})
  ENDIF(KWSTYLE_USE_MSVC_FORMAT)

  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Utilities/KWStyle/CMake.kws.xml.in
               ${CMake_BINARY_DIR}/CMake.kws.xml)
  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeMoreChecks.kws.xml.in
               ${CMake_BINARY_DIR}/CMakeMoreChecks.kws.xml)

  CONFIGURE_FILE(${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeFiles.txt.in
                 ${CMake_BINARY_DIR}/CMakeKWSFiles.txt)

  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMake_BINARY_DIR}/KWStyleReport.txt
    COMMAND ${KWSTYLE_EXECUTABLE}
    ARGS    -xml ${CMake_BINARY_DIR}/CMake.kws.xml -o ${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeOverwrite.txt -v ${KWSTYLE_ARGUMENTS} -D ${CMake_BINARY_DIR}/CMakeKWSFiles.txt
    COMMENT "Coding Style Checker"
    )

  ADD_CUSTOM_TARGET(MoreStyleChecks
    COMMAND ${KWSTYLE_EXECUTABLE}
    -xml ${CMake_BINARY_DIR}/CMakeMoreChecks.kws.xml -html ${CMake_BINARY_DIR}/html -o ${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeOverwrite.txt -v ${KWSTYLE_ARGUMENTS} -D ${CMake_BINARY_DIR}/CMakeKWSFiles.txt
    COMMENT "Coding Style Checker, more checks enabled"
    )

  ADD_CUSTOM_TARGET(StyleCheck DEPENDS ${CMake_BINARY_DIR}/KWStyleReport.txt)
ENDIF(CMAKE_USE_KWSTYLE)

