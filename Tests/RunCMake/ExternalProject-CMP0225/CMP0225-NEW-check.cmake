file(READ "${RunCMake_TEST_BINARY_DIR}/tmp/Sub-cfgcmd.txt" cfgcmd)
if(NOT cfgcmd MATCHES "CMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>")
  set(RunCMake_TEST_FAILED
    "Default configure command did not inject <INSTALL_DIR> prefix:\n${cfgcmd}")
endif()
