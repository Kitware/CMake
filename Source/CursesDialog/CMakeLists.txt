# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

add_executable(ccmake
  ccmake.cxx
  cmCursesBoolWidget.cxx
  cmCursesCacheEntryComposite.cxx
  cmCursesColor.cxx
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
set(CMAKE_REQUIRED_INCLUDES ${CURSES_INCLUDE_PATH})
target_link_libraries(ccmake CMakeLib)
if(CMAKE_USE_SYSTEM_FORM)
  find_path(CURSES_FORM_INCLUDE_DIR NAMES form.h HINTS ${CURSES_INCLUDE_PATH} ${CURSES_INCLUDE_PATH}/ncurses)
  if(CURSES_FORM_INCLUDE_DIR)
    target_include_directories(ccmake PRIVATE ${CURSES_FORM_INCLUDE_DIR})
    list(APPEND CMAKE_REQUIRED_INCLUDES ${CURSES_FORM_INCLUDE_DIR})
  endif()
  target_link_libraries(ccmake
    ${CURSES_FORM_LIBRARY}
    ${CURSES_LIBRARY}
  )
  set(CMAKE_REQUIRED_LIBRARIES
    ${CURSES_FORM_LIBRARY}
    ${CURSES_LIBRARY}
  )
  if(CURSES_EXTRA_LIBRARY)
    target_link_libraries(ccmake ${CURSES_EXTRA_LIBRARY})
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${CURSES_EXTRA_LIBRARY})
  endif()
else()
  target_link_libraries(ccmake cmForm)
  get_target_property(cmFormIncludeDirs cmForm INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND CMAKE_REQUIRED_INCLUDES ${cmFormIncludeDirs})
  get_target_property(cmFormLibraries cmForm INTERFACE_LINK_LIBRARIES)
  set(CMAKE_REQUIRED_LIBRARIES ${cmFormLibraries})
endif()

include(CheckSymbolExists)
check_symbol_exists(use_default_colors
  "form.h"
  HAVE_CURSES_USE_DEFAULT_COLORS)
if(HAVE_CURSES_USE_DEFAULT_COLORS)
  set_source_files_properties(cmCursesColor.cxx
    PROPERTIES COMPILE_DEFINITIONS HAVE_CURSES_USE_DEFAULT_COLORS)
endif()

if(CMake_JOB_POOL_LINK_BIN)
  set_property(TARGET ccmake PROPERTY JOB_POOL_LINK "link-bin")
endif()

CMake_OPTIONAL_COMPONENT(ccmake)
install(TARGETS ccmake DESTINATION ${CMAKE_BIN_DIR} ${COMPONENT})
