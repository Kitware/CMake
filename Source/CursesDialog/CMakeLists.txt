
SOURCE_FILES( CURSES_SRCS
              cmCursesBoolWidget
              cmCursesCacheEntryComposite
              cmCursesDummyWidget
              cmCursesFilePathWidget
              cmCursesForm
              cmCursesLabelWidget
              cmCursesMainForm
              cmCursesPathWidget
              cmCursesStringWidget
              cmCursesWidget
              ccurses
            )

INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source/CursesDialog/form)
INCLUDE_DIRECTORIES(${CURSES_INCLUDE_PATH})

LINK_LIBRARIES(${CURSES_LIBRARY})

IF(CURSES_EXTRA_LIBRARY)
  LINK_LIBRARIES(${CURSES_EXTRA_LIBRARY})
ENDIF(CURSES_EXTRA_LIBRARY)

LINK_LIBRARIES(cmForm)

ADD_EXECUTABLE(ccurses CURSES_SRCS)