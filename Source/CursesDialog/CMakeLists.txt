
SOURCE_FILES( CURSES_SRCS
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
INCLUDE_DIRECTORIES(${CURSES_INCLUDE_PATH})

LINK_LIBRARIES(${CURSES_LIBRARY})

IF(CURSES_EXTRA_LIBRARY)
  LINK_LIBRARIES(${CURSES_EXTRA_LIBRARY})
ENDIF(CURSES_EXTRA_LIBRARY)

LINK_LIBRARIES(cmForm)

ADD_EXECUTABLE(ccmake CURSES_SRCS)

INSTALL_TARGETS(/bin ccmake)