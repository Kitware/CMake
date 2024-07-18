if (NOT EXISTS ${INSTALL_MANIFEST})
  set(RunCMake_TEST_FAILED "Install manifest not generated: ${INSTALL_MANIFEST}")
endif()

file(STRINGS ${INSTALL_MANIFEST} lines ENCODING UTF-8)
list(LENGTH lines len)

if (NOT len EQUAL ${INSTALL_COUNT})
  set(RunCMake_TEST_FAILED "Install manifest missing content: ${len}/${INSTALL_COUNT}")
endif()
