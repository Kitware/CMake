set(CMAKE_RELEASE_DIRECTORY "/home/collab/itk/CMakeReleaseDirectory64" )
set(PROCESSORS 20)
set(CFLAGS "-64")
set(CXXFLAGS "-64")
set(LDFLAGS="-64")
set(HOST muse)
set(SCRIPT_NAME muse64)
set(MAKE_PROGRAM "gmake")
set(MAKE "${MAKE_PROGRAM} -j20")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
