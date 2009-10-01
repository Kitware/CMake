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
SUBDIRS(Doxygen KWStyle)

MAKE_DIRECTORY(${CMake_BINARY_DIR}/Docs)

# Add a documentation target.
SET(DOC_FILES "")

MACRO(ADD_DOCS target dependency)
  # Generate documentation for "ctest" executable.
  GET_TARGET_PROPERTY(CMD ${target} LOCATION)
  # only generate the documentation if the target is actually built
  IF(CMD)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${CMake_BINARY_DIR}/Docs/${target}.txt
      ${${target}-PATH} # Possibly set PATH, see below.
      COMMAND ${CMD}
      ARGS --help-full ${CMake_BINARY_DIR}/Docs/${target}.txt
           --help-full ${CMake_BINARY_DIR}/Docs/${target}.html
           --help-full ${CMake_BINARY_DIR}/Docs/${target}.1
           --help-full ${CMake_BINARY_DIR}/Docs/${target}.docbook
      DEPENDS ${target}
      MAIN_DEPENDENCY ${dependency}
      )
    INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES ${CMake_BINARY_DIR}/Docs/${target}.1)
    INSTALL_FILES(${CMAKE_DOC_DIR} FILES
      ${CMake_BINARY_DIR}/Docs/${target}.txt
      ${CMake_BINARY_DIR}/Docs/${target}.html
      ${CMake_BINARY_DIR}/Docs/${target}.docbook
      )
    SET(DOC_FILES ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/${target}.txt)
  ENDIF(CMD)
ENDMACRO(ADD_DOCS target dependency)

# Help cmake-gui find the Qt DLLs on Windows.
SET(WIN_SHELL_GENS "Visual Studio|NMake|MinGW|Watcom|Borland")
IF(BUILD_QtDialog AND "${CMAKE_GENERATOR}" MATCHES "${WIN_SHELL_GENS}"
    AND EXISTS "${QT_QMAKE_EXECUTABLE}" AND NOT CMAKE_NO_AUTO_QT_ENV)
  GET_FILENAME_COMPONENT(Qt_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
  IF(EXISTS "${Qt_BIN_DIR}/QtCore4.dll")
    # Tell the macro to set the path before running cmake-gui.
    STRING(REPLACE ";" "\\;" _PATH "PATH=${Qt_BIN_DIR};%PATH%")
    SET(cmake-gui-PATH COMMAND set "${_PATH}")
  ENDIF(EXISTS "${Qt_BIN_DIR}/QtCore4.dll")
ENDIF(BUILD_QtDialog AND "${CMAKE_GENERATOR}" MATCHES "${WIN_SHELL_GENS}"
  AND EXISTS "${QT_QMAKE_EXECUTABLE}" AND NOT CMAKE_NO_AUTO_QT_ENV)

# add the docs for the executables
ADD_DOCS(ctest      ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt)
ADD_DOCS(cpack      ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt)
ADD_DOCS(ccmake     ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt)
ADD_DOCS(CMakeSetup ${CMake_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in)
ADD_DOCS(cmake-gui ${CMake_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in)

# add the documentation for cmake itself

GET_TARGET_PROPERTY(CMD cmake LOCATION)
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMake_BINARY_DIR}/Docs/cmake.txt
  COMMAND ${CMD} 
  ARGS --copyright ${CMake_BINARY_DIR}/Docs/Copyright.txt
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.txt
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.html
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.1
       --help-full ${CMake_BINARY_DIR}/Docs/cmake.docbook
       --help-policies ${CMake_BINARY_DIR}/Docs/cmake-policies.txt
       --help-policies ${CMake_BINARY_DIR}/Docs/cmake-policies.html
       --help-policies ${CMake_BINARY_DIR}/Docs/cmakepolicies.1
       --help-properties ${CMake_BINARY_DIR}/Docs/cmake-properties.txt
       --help-properties ${CMake_BINARY_DIR}/Docs/cmake-properties.html
       --help-properties ${CMake_BINARY_DIR}/Docs/cmakeprops.1
       --help-variables ${CMake_BINARY_DIR}/Docs/cmake-variables.txt
       --help-variables ${CMake_BINARY_DIR}/Docs/cmake-variables.html
       --help-variables ${CMake_BINARY_DIR}/Docs/cmakevars.1
       --help-modules ${CMake_BINARY_DIR}/Docs/cmake-modules.txt
       --help-modules ${CMake_BINARY_DIR}/Docs/cmake-modules.html
       --help-modules ${CMake_BINARY_DIR}/Docs/cmakemodules.1
       --help-commands ${CMake_BINARY_DIR}/Docs/cmake-commands.txt
       --help-commands ${CMake_BINARY_DIR}/Docs/cmake-commands.html
       --help-commands ${CMake_BINARY_DIR}/Docs/cmakecommands.1
       --help-compatcommands ${CMake_BINARY_DIR}/Docs/cmake-compatcommands.txt
       --help-compatcommands ${CMake_BINARY_DIR}/Docs/cmake-compatcommands.html
       --help-compatcommands ${CMake_BINARY_DIR}/Docs/cmakecompat.1
  DEPENDS cmake
  MAIN_DEPENDENCY ${CMake_SOURCE_DIR}/Utilities/Doxygen/authors.txt
  )

INSTALL_FILES(${CMAKE_MAN_DIR}/man1 FILES
              ${CMake_BINARY_DIR}/Docs/cmake.1
              ${CMake_BINARY_DIR}/Docs/cmakecommands.1
              ${CMake_BINARY_DIR}/Docs/cmakecompat.1
              ${CMake_BINARY_DIR}/Docs/cmakeprops.1
              ${CMake_BINARY_DIR}/Docs/cmakepolicies.1
              ${CMake_BINARY_DIR}/Docs/cmakevars.1
              ${CMake_BINARY_DIR}/Docs/cmakemodules.1)

INSTALL_FILES(${CMAKE_DOC_DIR} FILES
    ${CMake_BINARY_DIR}/Docs/cmake.txt
    ${CMake_BINARY_DIR}/Docs/cmake.html
    ${CMake_BINARY_DIR}/Docs/cmake.docbook
    ${CMake_BINARY_DIR}/Docs/cmake-policies.txt
    ${CMake_BINARY_DIR}/Docs/cmake-policies.html
    ${CMake_BINARY_DIR}/Docs/cmake-properties.txt
    ${CMake_BINARY_DIR}/Docs/cmake-properties.html
    ${CMake_BINARY_DIR}/Docs/cmake-variables.txt
    ${CMake_BINARY_DIR}/Docs/cmake-variables.html 
    ${CMake_BINARY_DIR}/Docs/cmake-modules.txt
    ${CMake_BINARY_DIR}/Docs/cmake-modules.html
    ${CMake_BINARY_DIR}/Docs/cmake-commands.txt
    ${CMake_BINARY_DIR}/Docs/cmake-commands.html
    ${CMake_BINARY_DIR}/Docs/cmake-compatcommands.txt
    ${CMake_BINARY_DIR}/Docs/cmake-compatcommands.html )

# Drive documentation generation.
ADD_CUSTOM_TARGET(documentation ALL DEPENDS ${DOC_FILES} ${CMake_BINARY_DIR}/Docs/cmake.txt )
