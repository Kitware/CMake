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

INCLUDE_DIRECTORIES(${CURSES_INCLUDE_PATH})
LINK_LIBRARIES(${CURSES_LIBRARY} ${FORM_LIBRARY})

ADD_EXECUTABLE(ccurses CURSES_SRCS)