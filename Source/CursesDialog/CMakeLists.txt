SUBDIRS(form)

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
INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source/CursesDialog/form/)
LINK_LIBRARIES(${CURSES_LIBRARY})
LINK_LIBRARIES(cmForm)

ADD_EXECUTABLE(ccurses CURSES_SRCS)