
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

INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source/CursesDialog/form)
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Source/CursesDialog/form)
INCLUDE_DIRECTORIES(${CURSES_INCLUDE_PATH})




ADD_EXECUTABLE(ccmake ${CURSES_SRCS} )
TARGET_LINK_LIBRARIES(ccmake cmForm)
TARGET_LINK_LIBRARIES(ccmake ${CURSES_LIBRARY})
IF(CURSES_EXTRA_LIBRARY)
  TARGET_LINK_LIBRARIES(ccmake ${CURSES_EXTRA_LIBRARY})
ENDIF(CURSES_EXTRA_LIBRARY)

INSTALL_TARGETS(/bin ccmake)