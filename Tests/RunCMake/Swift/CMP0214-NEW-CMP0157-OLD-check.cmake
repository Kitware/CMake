if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(path "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/impl-Debug.ninja")
else()
  set(path "${RunCMake_TEST_BINARY_DIR}/build.ninja")
endif()
file(READ "${path}" build_ninja)

# With CMP0157 OLD (no split build), CMAKE_SHARED_LINKER_FLAGS should NOT be
# passed to Swift shared libraries.
if(build_ninja MATCHES "Swift_SHARED_LIBRARY_LINKER[^\n]*(\n  [^\n]+)*\n  LINK_FLAGS = [^\n]*-foo")
  string(APPEND RunCMake_TEST_FAILED "Build file:\n  ${path}\nunexpectedly has -foo in LINK_FLAGS for Swift shared library\n")
endif()

# With CMP0157 OLD (no split build), CMAKE_EXE_LINKER_FLAGS should NOT be
# passed to Swift executables even with CMP0214 NEW.
if(build_ninja MATCHES "Swift_EXECUTABLE_LINKER[^\n]*(\n  [^\n]+)*\n  LINK_FLAGS = [^\n]*-foo")
  string(APPEND RunCMake_TEST_FAILED "Build file:\n  ${path}\nunexpectedly has -foo in LINK_FLAGS for Swift executable\n")
endif()
