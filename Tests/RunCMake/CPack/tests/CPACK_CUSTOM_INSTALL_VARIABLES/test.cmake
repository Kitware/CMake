set(CPACK_CUSTOM_INSTALL_VARIABLES "FOO=foo.txt" "BAR=bar.txt")

install(CODE [[
  file(WRITE ${CMAKE_INSTALL_PREFIX}/foo/${FOO})
  file(WRITE ${CMAKE_INSTALL_PREFIX}/foo/${BAR})
  file(WRITE ${CMAKE_INSTALL_PREFIX}/foo/baz.txt)
]])
