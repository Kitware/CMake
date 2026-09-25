file(READ "${RunCMake_TEST_BINARY_DIR}/tmp/Sub-cfgcmd.txt" cfgcmd)
if(cfgcmd MATCHES "CMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>")
  set(RunCMake_TEST_FAILED
    "Unset policy unexpectedly injected <INSTALL_DIR> prefix:\n${cfgcmd}")
endif()
