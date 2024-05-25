cmake_policy(SET CMP0169 OLD)

include(FetchContent)

# Need to see the download command output
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -P
                   ${CMAKE_CURRENT_LIST_DIR}/countArgs.cmake
                   before "" after
)

FetchContent_Populate(t1)
