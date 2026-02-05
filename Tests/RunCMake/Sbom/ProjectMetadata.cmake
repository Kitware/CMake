include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(test INTERFACE)

install(
  TARGETS test
  EXPORT test_targets
  DESTINATION .
)
