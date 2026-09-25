file(READ "${RunCMake_TEST_BINARY_DIR}/tmp/Sub-cfgcmd.txt" cfgcmd)
if(cfgcmd MATCHES "CMAKE_INSTALL_PREFIX")
  set(RunCMake_TEST_FAILED
    "Empty configure command should not receive a prefix:\n${cfgcmd}")
endif()
