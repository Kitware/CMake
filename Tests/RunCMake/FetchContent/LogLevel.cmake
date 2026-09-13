cmake_policy(SET CMP0168 NEW)

include(FetchContent)

set(patch_count_file "${CMAKE_CURRENT_BINARY_DIR}/patch-count.txt")

FetchContent_Declare(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E make_directory <SOURCE_DIR>
  PATCH_COMMAND ${CMAKE_COMMAND}
    -DPATCH_COUNT_FILE=${patch_count_file}
    -P ${CMAKE_CURRENT_LIST_DIR}/LogLevelPatch.cmake
)
FetchContent_MakeAvailable(t1)

file(READ "${patch_count_file}" patch_count)
if(NOT patch_count EQUAL 1)
  message(SEND_ERROR "Patch step ran ${patch_count} times, expected once")
endif()
