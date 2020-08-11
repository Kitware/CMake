include(ExternalProject)

set(script "${CMAKE_CURRENT_LIST_DIR}/countArgs.cmake")
ExternalProject_Add(
  blankChecker
  DOWNLOAD_COMMAND  ${CMAKE_COMMAND} -P "${script}" download  "" after
  UPDATE_COMMAND    ${CMAKE_COMMAND} -P "${script}" update    "" after
  PATCH_COMMAND     ${CMAKE_COMMAND} -P "${script}" patch     "" after
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -P "${script}" configure "" after
  BUILD_COMMAND     ${CMAKE_COMMAND} -P "${script}" build     "" after
  INSTALL_COMMAND   ${CMAKE_COMMAND} -P "${script}" install   "" after
  TEST_COMMAND      ${CMAKE_COMMAND} -P "${script}" test      "" after
)
