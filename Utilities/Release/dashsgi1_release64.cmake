set(CMAKE_RELEASE_DIRECTORY "/usr/people/kitware/CMakeReleaseDirectory64")
set(PROCESSORS 2)
set(CFLAGS "-64")
set(CXXFLAGS "-64")
set(LDFLAGS="-64")
set(HOST dashsgi1)
set(SCRIPT_NAME dashsgi164)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -P")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CPACK_SYSTEM_NAME:STRING=IRIX64-64
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
