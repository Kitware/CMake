cmake_policy(SET CMP0168 NEW)  # Faster, don't need to test with sub-build

include(FetchContent)

FetchContent_Populate(t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Direct download"
  UPDATE_COMMAND ""
)

FetchContent_Declare(t2
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Declared download"
  UPDATE_COMMAND ""
)
FetchContent_Populate(t2)
