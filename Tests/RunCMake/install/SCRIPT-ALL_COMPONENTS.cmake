
install(
  SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/install_script.cmake"
  ALL_COMPONENTS
)

install(
  CODE "write_empty_file(empty2.txt)"
  ALL_COMPONENTS
  EXCLUDE_FROM_ALL
)

install(
  CODE "write_empty_file(empty3.txt)"
  ALL_COMPONENTS
)
