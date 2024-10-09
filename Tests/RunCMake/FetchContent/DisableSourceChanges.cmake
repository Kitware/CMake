cmake_policy(SET CMP0168 NEW)

# Undocumented variable used to catch attempts to write to anywhere under the
# source directory that isn't under the build directory. In order for this
# code path to be checked for direct population mode, we need a non-empty
# download, update, or patch command so that the population code path is used.
# Custom commands might not write to the source directory and instead just
# print messages or other non-modifying tasks, like is done here.
set(CMAKE_DISABLE_SOURCE_CHANGES TRUE)

include(FetchContent)

FetchContent_Declare(
  WithProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WithProject   # This exists
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Download command executed"
)
FetchContent_MakeAvailable(WithProject)
