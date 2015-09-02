set(PROCESSORS 1)
set(BOOTSTRAP_ARGS "--docdir=doc/cmake")
set(HOST magrathea)
set(MAKE_PROGRAM "make")
set(CC gcc332s)
set(CXX c++332s)
set(CFLAGS   -DDT_RUNPATH=29)
set(CXXFLAGS -DDT_RUNPATH=29)
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CURSES_LIBRARY:FILEPATH=/usr/i686-gcc-332s/lib/libncurses.a
CURSES_INCLUDE_PATH:PATH=/usr/i686-gcc-332s/include/ncurses
FORM_LIBRARY:FILEPATH=/usr/i686-gcc-332s/lib/libform.a
CMAKE_USE_OPENSSL:BOOL=ON
OPENSSL_CRYPTO_LIBRARY:FILEPATH=/home/kitware/openssl-1.0.2d/lib/libcrypto.a
OPENSSL_INCLUDE_DIR:PATH=/home/kitware/openssl-1.0.2d/include
OPENSSL_SSL_LIBRARY:FILEPATH=/home/kitware/openssl-1.0.2d/lib/libssl.a
CPACK_SYSTEM_NAME:STRING=Linux-i386
BUILD_QtDialog:BOOL:=TRUE
CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL:BOOL=TRUE
CMake_INSTALL_DEPENDENCIES:BOOL=ON
QT_QMAKE_EXECUTABLE:FILEPATH=/home/kitware/qt-4.43-install/bin/qmake
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
