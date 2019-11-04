check_libraries(
  ".*/Tests/RunCMake/INSTALL_NAME_DIR/prefix_genex-build/fake_install/lib/"
  ".*/Tests/RunCMake/INSTALL_NAME_DIR/prefix_genex-build/real_install/lib/"
  # "$" has to be escaped twice because of its significance in regexes.
  "\\\${_IMPORT_PREFIX}/lib/"
  )
