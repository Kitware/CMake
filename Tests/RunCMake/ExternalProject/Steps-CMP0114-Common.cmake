include(ExternalProject)

ExternalProject_Add(proj0
  SOURCE_DIR "."
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj0-download-mark
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj0-configure-mark
  BUILD_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj0-build-mark
  INSTALL_COMMAND ""
  )

cmake_policy(GET CMP0114 cmp0114)
if(cmp0114 STREQUAL "NEW")
  set(step_targets "update;test")
  set(independent_step_targets "")
else()
  set(step_targets "install;test")
  set(independent_step_targets "download;update")
endif()

ExternalProject_Add(proj1
  DEPENDS proj0
  SOURCE_DIR "."
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-download-mark
  UPDATE_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-update-mark
  PATCH_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-patch-mark
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-configure-mark
  BUILD_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-build-mark
  INSTALL_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-install-mark
  TEST_COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/proj1-test-mark
  TEST_EXCLUDE_FROM_MAIN 1 # Along with 'STEP_TARGETS test', implies 'STEP_TARGETS install'
  UPDATE_DISCONNECTED 1 # Along with 'STEP_TARGETS update', implies 'STEP_TARGETS download'
  STEP_TARGETS ${step_targets}
  INDEPENDENT_STEP_TARGETS ${independent_step_targets}
  )
