if(NOT ${RunCMake_SUBTEST_SUFFIX} STREQUAL "fail")
    set(MULTIARCH_control "Multi-Arch: ${RunCMake_SUBTEST_SUFFIX}")
    verifyDebControl("${FOUND_FILE_1}" "MULTIARCH" "control")
endif ()
