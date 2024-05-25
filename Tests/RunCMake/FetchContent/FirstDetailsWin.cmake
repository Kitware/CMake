cmake_policy(SET CMP0169 OLD)

include(FetchContent)

# Need to see the download command output
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "First details used"
)

FetchContent_Declare(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Second details used"
)

FetchContent_Populate(t1)
