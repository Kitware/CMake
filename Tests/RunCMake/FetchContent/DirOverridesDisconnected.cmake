include(FetchContent)

# Test using saved details. We are re-using a SOURCE_DIR from a previous test
# so the download command should not be executed.
FetchContent_Declare(
  t1
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/savedSrc
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/savedBin
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E false
)
FetchContent_Populate(t1)

if(NOT "${t1_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/savedSrc")
  message(FATAL_ERROR "Wrong SOURCE_DIR returned: ${t1_SOURCE_DIR}")
endif()
if(NOT "${t1_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/savedBin")
  message(FATAL_ERROR "Wrong BINARY_DIR returned: ${t1_BINARY_DIR}")
endif()
