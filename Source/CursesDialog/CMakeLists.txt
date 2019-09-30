# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

add_executable(ccmake
  ccmake.cxx
  cmCursesBoolWidget.cxx
  cmCursesCacheEntryComposite.cxx
  cmCursesDummyWidget.cxx
  cmCursesFilePathWidget.cxx
  cmCursesForm.cxx
  cmCursesLabelWidget.cxx
  cmCursesLongMessageForm.cxx
  cmCursesMainForm.cxx
  cmCursesOptionsWidget.cxx
  cmCursesPathWidget.cxx
  cmCursesStringWidget.cxx
  cmCursesWidget.cxx
  )
target_include_directories(ccmake PRIVATE ${CURSES_INCLUDE_PATH})
target_link_libraries(ccmake CMakeLib)
if(CMAKE_USE_SYSTEM_FORM)
  find_path(CURSES_FORM_INCLUDE_DIR NAMES form.h HINTS ${CURSES_INCLUDE_PATH} ${CURSES_INCLUDE_PATH}/ncurses)
  if(CURSES_FORM_INCLUDE_DIR)
    target_include_directories(ccmake PRIVATE ${CURSES_FORM_INCLUDE_DIR})
  endif()
  target_link_libraries(ccmake
    ${CURSES_FORM_LIBRARY}
    ${CURSES_LIBRARY}
  )
  if(CURSES_EXTRA_LIBRARY)
    target_link_libraries(ccmake ${CURSES_EXTRA_LIBRARY})
  endif()
else()
  target_link_libraries(ccmake cmForm)
endif()

CMake_OPTIONAL_COMPONENT(ccmake)
install(TARGETS ccmake DESTINATION ${CMAKE_BIN_DIR} ${COMPONENT})
