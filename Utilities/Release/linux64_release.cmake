set(PROCESSORS 4)
set(BOOTSTRAP_ARGS "--docdir=doc/cmake")
set(HOST linux64)
set(MAKE_PROGRAM "make")
set(CC /opt/gcc-4.9.2/bin/gcc)
set(CXX /opt/gcc-4.9.2/bin/g++)
set(CFLAGS   "")
set(CXXFLAGS "")
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CURSES_LIBRARY:FILEPATH=/home/kitware/ncurses-5.9/lib/libncurses.a
CURSES_INCLUDE_PATH:PATH=/home/kitware/ncurses-5.9/include
FORM_LIBRARY:FILEPATH=/home/kitware/ncurses-5.9/lib/libform.a
CMAKE_USE_OPENSSL:BOOL=ON
OPENSSL_CRYPTO_LIBRARY:FILEPATH=/home/kitware/openssl-1.0.2d/lib/libcrypto.a
OPENSSL_INCLUDE_DIR:PATH=/home/kitware/openssl-1.0.2d/include
OPENSSL_SSL_LIBRARY:FILEPATH=/home/kitware/openssl-1.0.2d/lib/libssl.a
CPACK_SYSTEM_NAME:STRING=Linux-x86_64
BUILD_QtDialog:BOOL:=TRUE
CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL:BOOL=TRUE
CMake_INSTALL_DEPENDENCIES:BOOL=ON
QT_QMAKE_EXECUTABLE:FILEPATH=/home/kitware/qt-4.8.6/bin/qmake
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
