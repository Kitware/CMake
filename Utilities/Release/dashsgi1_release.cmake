set(CMAKE_RELEASE_DIRECTORY "/usr/people/kitware/CMakeReleaseDirectory")
set(PROCESSORS 2)
set(HOST dashsgi1)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -P")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CPACK_SYSTEM_NAME:STRING=IRIX64-n32
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
