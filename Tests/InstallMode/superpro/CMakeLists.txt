# This CMakeLists.txt is part of the superproject (add_subdirectory).

# Below file transfers are executed at configuration time!

file(
  COPY
    "file_copy.txt"
  DESTINATION
    "${CMAKE_INSTALL_PREFIX}"
)

file(COPY_FILE
  "${CMAKE_CURRENT_SOURCE_DIR}/file_copy_file.txt"
  "${CMAKE_INSTALL_PREFIX}/file_copy_file.txt"
)

file(
  INSTALL
    "file_install.txt"
  DESTINATION
    "${CMAKE_INSTALL_PREFIX}"
)

file(
  CREATE_LINK
    "${CMAKE_CURRENT_SOURCE_DIR}/file_create_link_symbolic.txt"
    "${CMAKE_INSTALL_PREFIX}/file_create_link_symbolic.txt"
  SYMBOLIC
)
