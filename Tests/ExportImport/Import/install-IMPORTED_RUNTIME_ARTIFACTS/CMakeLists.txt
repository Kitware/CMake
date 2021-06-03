# Import targets from the exported build tree.
include(${Import_BINARY_DIR}/../Export/ExportBuildTree.cmake)

set(_tgts
  bld_testExe1 # Ordinary executable
  bld_testExe2lib # Ordinary shared library
  bld_testLib3 # Shared library with version and soversion
  bld_testLib4 # Framework library
  bld_testExe3 # Bundle executable
  bld_testMod1 # Module library
  bld_testMod2 # CFBundle
  )

install(IMPORTED_RUNTIME_ARTIFACTS ${_tgts}
  RUNTIME DESTINATION aaa/executables
  LIBRARY DESTINATION aaa/libraries
  BUNDLE DESTINATION aaa/bundles
  FRAMEWORK DESTINATION aaa/frameworks
  )

install(IMPORTED_RUNTIME_ARTIFACTS ${_tgts}
  DESTINATION bbb
  )

install(IMPORTED_RUNTIME_ARTIFACTS ${_tgts}
  BUNDLE DESTINATION zzz/bundles
  FRAMEWORK DESTINATION zzz/frameworks
  )

install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/check_installed.cmake")
