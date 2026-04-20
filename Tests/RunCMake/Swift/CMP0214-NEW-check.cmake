if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(path "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/impl-Debug.ninja")
else()
  set(path "${RunCMake_TEST_BINARY_DIR}/build.ninja")
endif()
file(READ "${path}" build_ninja)

# With CMP0157 NEW (split build), CMAKE_SHARED_LINKER_FLAGS should be passed
# to Swift shared libraries regardless of CMP0214.
if(NOT build_ninja MATCHES "Swift_SHARED_LIBRARY_LINKER[^\n]*(\n  [^\n]+)*\n  LINK_FLAGS = [^\n]*-foo")
  string(APPEND RunCMake_TEST_FAILED "Build file:\n  ${path}\ndoes not have -foo in LINK_FLAGS for Swift shared library\n")
endif()

# With CMP0214 NEW, CMAKE_EXE_LINKER_FLAGS should be passed to Swift executables.
if(NOT build_ninja MATCHES "Swift_EXECUTABLE_LINKER[^\n]*(\n  [^\n]+)*\n  LINK_FLAGS = [^\n]*-foo")
  string(APPEND RunCMake_TEST_FAILED "Build file:\n  ${path}\ndoes not have -foo in LINK_FLAGS for Swift executable\n")
endif()
