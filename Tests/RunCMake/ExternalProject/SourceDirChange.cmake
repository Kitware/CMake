include(ExternalProject)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/first")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/second")

if("${SOURCE_DIR_CHANGE}" STREQUAL "")
  set(source_dir first)
else()
  set(source_dir second)
endif()

ExternalProject_Add(source_dir_change
  SOURCE_DIR        "${CMAKE_BINARY_DIR}/${source_dir}"
  DOWNLOAD_COMMAND  "${CMAKE_COMMAND}" -E echo "Download command executed"
  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  TEST_COMMAND      ""
  INSTALL_COMMAND   ""
)
