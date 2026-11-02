project(bar SPDX_LICENSE BSD-3)

set(bar_SPDX_LICENSE MIT)
set(bar_DESCRIPTION "Description from project variables")
set(PROJECT_NAME overridden-parent-name)
set(PROJECT_SPDX_LICENSE GPL-3.0-only)
set(PROJECT_DESCRIPTION "Generic description must not be used")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ProjectLicenseSubdir
  ${CMAKE_CURRENT_BINARY_DIR}/licensed-subdir)

add_library(bar INTERFACE)
install(
  TARGETS bar
  EXPORT bar_targets
  DESTINATION .
)
