set(PROCESSORS 1)
set(HOST magrathea)
set(MAKE_PROGRAM "make")
set(CC gcc332s)
set(CXX c++332s)
set(GIT_COMMAND /home/kitware/.userroot/git/bin/git)
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CURSES_LIBRARY:FILEPATH=/usr/i686-gcc-332s/lib/libncurses.a
CURSES_INCLUDE_PATH:PATH=/usr/i686-gcc-332s/include/ncurses
FORM_LIBRARY:FILEPATH=/usr/i686-gcc-332s/lib/libform.a
CPACK_SYSTEM_NAME:STRING=Linux-i386
BUILD_QtDialog:BOOL:=TRUE
QT_QMAKE_EXECUTABLE:FILEPATH=/home/kitware/qt-x11-opensource-src-4.3.3-install/bin/qmake
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
