set(CMAKE_RELEASE_DIRECTORY "c:/cygwin/home/dashboard/CMakeReleaseDirectory")
set(CONFIGURE_WITH_CMAKE TRUE)
set(CMAKE_CONFIGURE_PATH "c:/Program\\ Files\\ \\(x86\\)/CMake/bin/cmake.exe")
set(PROCESSORS 8)
set(HOST dash2win64)
set(CPACK_BINARY_GENERATORS "NSIS ZIP")
set(CPACK_SOURCE_GENERATORS "ZIP")
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j8")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_DOC_DIR:STRING=doc/cmake
CMAKE_USE_OPENSSL:BOOL=OFF
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CMAKE_Fortran_COMPILER:FILEPATH=FALSE
CMAKE_GENERATOR:INTERNAL=Unix Makefiles
BUILD_QtDialog:BOOL:=TRUE
CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL:BOOL=TRUE
CMake_INSTALL_DEPENDENCIES:BOOL=ON
QT_QMAKE_EXECUTABLE:FILEPATH=c:/Dashboards/Support/qt-build/Qt/bin/qmake.exe
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(GIT_EXTRA "git config core.autocrlf true")
include(${path}/release_cmake.cmake)
