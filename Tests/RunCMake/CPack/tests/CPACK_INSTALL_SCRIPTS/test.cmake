file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/abc.txt" "test content")
set(user_script_ "${CMAKE_CURRENT_BINARY_DIR}/user-script.cmake")
file(WRITE "${user_script_}"
  "file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/foo\"
    TYPE FILE FILES \"${CMAKE_CURRENT_BINARY_DIR}/abc.txt\")")

if(RunCMake_SUBTEST_SUFFIX STREQUAL "both")
    set(CPACK_INSTALL_SCRIPT "${user_script_}")
    set(CPACK_INSTALL_SCRIPTS "${CPACK_INSTALL_SCRIPT}")

elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "singular")
    set(CPACK_INSTALL_SCRIPT "${user_script_}")

elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "plural")
    set(CPACK_INSTALL_SCRIPTS "${user_script_}")

else()
    message(FATAL_ERROR "Unexpected subtest name: ${RunCMake_SUBTEST_SUFFIX}")

endif()

function(run_after_include_cpack)
  file(READ "${CPACK_OUTPUT_CONFIG_FILE}" conf_file_)
  string(REGEX REPLACE "SET\\(CPACK_INSTALL_CMAKE_PROJECTS [^)]*\\)" "" conf_file_ "${conf_file_}")
  file(WRITE "${CPACK_OUTPUT_CONFIG_FILE}" "${conf_file_}")
endfunction()
