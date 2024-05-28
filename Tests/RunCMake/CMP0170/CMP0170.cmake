cmake_policy(SET CMP0168 NEW)  # Faster, don't need to test with sub-build
cmake_policy(SET CMP0169 OLD)  # So we can test FetchContent_Populate() directly

set(FETCHCONTENT_FULLY_DISCONNECTED TRUE)

include(FetchContent)

message(STATUS "Starting population")

FetchContent_Declare(t1
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/IdoNotExist"
)
FetchContent_Populate(t1)

FetchContent_Declare(t2
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/IdoNotExist"
)
FetchContent_MakeAvailable(t2)
