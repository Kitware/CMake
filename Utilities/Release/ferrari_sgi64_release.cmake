set(CVS_COMMAND "/home/whoffman/bin/cvs")
set(CMAKE_RELEASE_DIRECTORY "/home/whoffman/CMakeReleaseDirectory64")
set(PROCESSORS 2)
set(CFLAGS "-64")
set(FFLAGS "-64")
set(CXXFLAGS "-64")
set(LDFLAGS="-64")
set(HOST sgi)
set(SCRIPT_NAME sgi64)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -P")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CPACK_SYSTEM_NAME:STRING=IRIX64-64
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
