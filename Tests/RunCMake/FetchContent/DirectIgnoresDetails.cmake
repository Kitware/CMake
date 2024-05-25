cmake_policy(SET CMP0169 OLD)

include(FetchContent)

FetchContent_Declare(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Saved details used"
)

# No QUIET option given, so command output will be shown
FetchContent_Populate(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Local details used"
)
