if(NOT CPackComponentsDEB_SOURCE_DIR)
  message(FATAL_ERROR "CPackComponentsDEB_SOURCE_DIR not set")
endif()

include(${CPackComponentsDEB_SOURCE_DIR}/RunCPackVerifyResult.cmake)


set(actual_output)
run_cpack(actual_output
          CPack_output
          CPack_error
          EXPECT_FAILURE
          CONFIG_ARGS ${config_args}
          CONFIG_VERBOSE ${config_verbose})

string(REGEX MATCH "dpkg-shlibdeps: error: (cannot|couldn't) find library\n[ \t]*libmyprivatelib.so.1 needed by ./usr/bin/mylibapp3" expected_error ${CPack_error})
if(NOT expected_error)
  message(FATAL_ERROR "Did not get the expected error-message!")
endif()
