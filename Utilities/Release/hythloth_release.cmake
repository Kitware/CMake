set(PROCESSORS 2)
set(HOST hythloth)
set(MAKE_COMMAND "make")
set(MAKE "${MAKE_COMMAND} -j2")
set(INITIAL_CACHE "
CMAKE_BUILD_TYPE:STRING=Release
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
