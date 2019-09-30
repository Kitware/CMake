# pseudo find_module

if (REASON_FAILURE_MESSAGE)
  list (PREPEND REASON_FAILURE_MESSAGE "REASON_FAILURE_MESSAGE")
endif()

include(FindPackageHandleStandardArgs)

if (CONFIG_MODE)
  find_package (CustomMessage QUIET CONFIG HINTS "${CMAKE_MODULE_PATH}")
  find_package_handle_standard_args(CustomMessage CONFIG_MODE
                                                  ${REASON_FAILURE_MESSAGE})
else()
  find_package_handle_standard_args(CustomMessage REQUIRED_VARS FOOBAR
                                                  VERSION_VAR CustomMessage_VERSION
                                                  ${REASON_FAILURE_MESSAGE})
endif()
