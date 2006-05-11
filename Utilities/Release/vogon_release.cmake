set(CMAKE_RELEASE_DIRECTORY "c:/hoffman/CMakeReleaseDirectory")
set(CONFIGURE_WITH_CMAKE TRUE)
set(CMAKE_CONFIGURE_PATH "c:/Hoffman/My\\ Builds/CMakeVSNMake71Dev/bin/cmake.exe")
set(PROCESSORS 1)
set(HOST vogon)
set(CPACK_BINARY_GENERATORS "NSIS ZIP")
set(CPACK_SOURCE_GENERATORS "ZIP")
set(MAKE_PROGRAM "nmake")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CMAKE_GENERATOR:INTERNAL=NMake Makefiles
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
