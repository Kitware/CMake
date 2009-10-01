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

SET( CURSES_SRCS
              CursesDialog/cmCursesBoolWidget
              CursesDialog/cmCursesCacheEntryComposite
              CursesDialog/cmCursesDummyWidget
              CursesDialog/cmCursesFilePathWidget
              CursesDialog/cmCursesForm
              CursesDialog/cmCursesLabelWidget
              CursesDialog/cmCursesLongMessageForm
              CursesDialog/cmCursesMainForm
              CursesDialog/cmCursesPathWidget
              CursesDialog/cmCursesStringWidget
              CursesDialog/cmCursesWidget
              CursesDialog/ccmake
   )

INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source/CursesDialog/form
                    ${CMake_BINARY_DIR}/Source/CursesDialog/form)
INCLUDE_DIRECTORIES(${CURSES_INCLUDE_PATH})


ADD_EXECUTABLE(ccmake ${CURSES_SRCS} )
TARGET_LINK_LIBRARIES(ccmake CMakeLib)
TARGET_LINK_LIBRARIES(ccmake cmForm)

INSTALL_TARGETS(/bin ccmake)
