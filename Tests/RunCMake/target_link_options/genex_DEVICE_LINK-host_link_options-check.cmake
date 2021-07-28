
if (NOT actual_stdout MATCHES "-Xlinker=OPT1 -Xlinker=OPT2 -Xlinker=OPT3 -Xlinker=OPT4 -Xlinker=OPT5")
    set (RunCMake_TEST_FAILED "Not found expected '-Xlinker=OPT1 -Xlinker=OPT2 -Xlinker=OPT3 -Xlinker=OPT4 -Xlinker=OPT5'.")
endif()
