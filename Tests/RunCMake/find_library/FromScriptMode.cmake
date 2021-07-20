
if(TEMP_DIR)
  file(REMOVE_RECURSE "${TEMP_DIR}")
  file(MAKE_DIRECTORY "${TEMP_DIR}")
  file(MAKE_DIRECTORY "${TEMP_DIR}/lib")
  file(WRITE "${TEMP_DIR}/lib/libcreated.a" "created")
endif()

set(CMAKE_FIND_DEBUG_MODE 1)
find_library(CREATED_LIBRARY NAMES library_no_exist)

set(CMAKE_PREFIX_PATH "${TEMP_DIR}")
find_library(CREATED_LIBRARY NAMES created)
message(STATUS "CREATED_LIBRARY='${CREATED_LIBRARY}'")
set(CMAKE_FIND_DEBUG_MODE 0)
