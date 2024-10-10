# We're providing a pre-existing source directory. Make sure we don't trigger
# an error if the undocumented but used-in-the-wild CMAKE_DISABLE_SOURCE_CHANGES
# variable is set.
set(CMAKE_DISABLE_SOURCE_CHANGES TRUE)

include(ExternalProject)

ExternalProject_Add(source_dir_existing
  SOURCE_DIR        "${CMAKE_CURRENT_LIST_DIR}/Foo"
  DOWNLOAD_COMMAND  "${CMAKE_COMMAND}" -E echo "Download command executed"
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  TEST_COMMAND      ""
  INSTALL_COMMAND   ""
)
