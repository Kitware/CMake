set(CMAKE_RELEASE_DIRECTORY "c:/hoffman/CMakeReleaseDirectory")
set(CONFIGURE_WITH_CMAKE TRUE)
set(CMAKE_CONFIGURE_PATH "c:/Program\\ Files/CMake\\ 2.6/bin/cmake.exe")
set(PROCESSORS 1)
set(HOST vogon)
set(CPACK_BINARY_GENERATORS "NSIS ZIP")
set(CPACK_SOURCE_GENERATORS "ZIP")
set(MAKE_PROGRAM "nmake")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CMAKE_GENERATOR:INTERNAL=NMake Makefiles
CMAKE_MT_EXECUTABLE:STRING=mt
QT_QMAKE_EXECUTABLE:FILEPATH=C:/QT/qt-win-opensource-src-4.5.0/bin/qmake.exe
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
