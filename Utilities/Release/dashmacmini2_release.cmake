set(PROCESSORS 2)
set(HOST dashmacmini2)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j2")
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_OSX_ARCHITECTURES:STRING=ppc\;i386
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
")
set(INSTALLER_SUFFIX "*.dmg")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
