project(child SPDX_LICENSE Apache-2.0 LANGUAGES NONE)

set(PROJECT_NAME overridden-child-name)
set(PROJECT_SPDX_LICENSE GPL-3.0-only)

add_library(child INTERFACE)
install(
  TARGETS child
  EXPORT child_targets
  DESTINATION .
)
