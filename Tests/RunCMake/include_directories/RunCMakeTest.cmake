include(RunCMake)

run_cmake(NotFoundContent)
run_cmake(DebugIncludes)
run_cmake(TID-bad-target)
run_cmake(SourceDirectoryInInterface)
run_cmake(BinaryDirectoryInInterface)
run_cmake(RelativePathInInterface)
run_cmake(ImportedTarget)
run_cmake(RelativePathInGenex)
run_cmake(CMP0021)
run_cmake(install_config)
run_cmake(incomplete-genex)
run_cmake(export-NOWARN)

configure_file(
  "${RunCMake_SOURCE_DIR}/CMakeLists.txt"
  "${RunCMake_BINARY_DIR}/copy/CMakeLists.txt"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/empty.cpp"
  "${RunCMake_BINARY_DIR}/copy/empty.cpp"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/SourceDirectoryInInterface.cmake"
  "${RunCMake_BINARY_DIR}/copy/SourceDirectoryInInterface.cmake"
  COPYONLY
)
set(RunCMake_TEST_OPTIONS "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/copy/SourceDirectoryInInterface/prefix")
set(RunCMake_TEST_FILE "${RunCMake_BINARY_DIR}/copy/SourceDirectoryInInterface")
set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/copy")
run_cmake(InstallInSrcDir)
unset(RunCMake_TEST_SOURCE_DIR)
unset(RunCMake_TEST_FILE)

set(RunCMake_TEST_OPTIONS "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/InstallInBinDir-build/prefix")
set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/InstallInBinDir-build")
set(RunCMake_TEST_FILE "${RunCMake_SOURCE_DIR}/BinaryDirectoryInInterface")
run_cmake(InstallInBinDir)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_FILE)

configure_file(
  "${RunCMake_SOURCE_DIR}/CMakeLists.txt"
  "${RunCMake_BINARY_DIR}/prefix/src/CMakeLists.txt"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/empty.cpp"
  "${RunCMake_BINARY_DIR}/prefix/src/empty.cpp"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/SourceDirectoryInInterface.cmake"
  "${RunCMake_BINARY_DIR}/prefix/src/SourceDirectoryInInterface.cmake"
  COPYONLY
)

foreach(policyStatus "" NEW OLD)
  if (NOT "${policyStatus}" STREQUAL "")
    set(policyOption -DCMAKE_POLICY_DEFAULT_CMP0052=${policyStatus})
  else()
    unset(policyOption)
    set(policyStatus WARN)
  endif()
  set(RunCMake_TEST_OPTIONS "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/prefix" ${policyOption})
  # Set the RunCMake_TEST_SOURCE_DIR here to the copy too. This is needed to run
  # the test suite in-source properly.  Otherwise the install directory would be
  # a subdirectory or the source directory, which is allowed and tested separately
  # below.
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/prefix/src")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/prefix/BinInInstallPrefix-CMP0052-${policyStatus}-build")
  set(RunCMake_TEST_FILE "${RunCMake_SOURCE_DIR}/BinaryDirectoryInInterface")
  run_cmake(BinInInstallPrefix-CMP0052-${policyStatus})
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_FILE)

  set(RunCMake_TEST_FILE "${RunCMake_BINARY_DIR}/prefix/src/SourceDirectoryInInterface")
  run_cmake(SrcInInstallPrefix-CMP0052-${policyStatus})
  unset(RunCMake_TEST_SOURCE_DIR)
  unset(RunCMake_TEST_FILE)
endforeach()

set(RunCMake_TEST_OPTIONS "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/InstallPrefixInInterface-build/prefix")
run_cmake(InstallPrefixInInterface)

configure_file(
  "${RunCMake_SOURCE_DIR}/CMakeLists.txt"
  "${RunCMake_BINARY_DIR}/installToSrc/CMakeLists.txt"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/empty.cpp"
  "${RunCMake_BINARY_DIR}/installToSrc/empty.cpp"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/InstallPrefixInInterface.cmake"
  "${RunCMake_BINARY_DIR}/installToSrc/InstallPrefixInInterface.cmake"
  COPYONLY
)
set(RunCMake_TEST_OPTIONS "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/installToSrc/InstallPrefixInInterface/prefix")
set(RunCMake_TEST_FILE "${RunCMake_BINARY_DIR}/installToSrc/InstallPrefixInInterface")
set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/installToSrc")
run_cmake(InstallToPrefixInSrcDirOutOfSource)
unset(RunCMake_TEST_SOURCE_DIR)
unset(RunCMake_TEST_FILE)


file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}/installToSrcInSrc")
set(RunCMake_TEST_NO_CLEAN ON)

configure_file(
  "${RunCMake_SOURCE_DIR}/CMakeLists.txt"
  "${RunCMake_BINARY_DIR}/installToSrcInSrc/CMakeLists.txt"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/empty.cpp"
  "${RunCMake_BINARY_DIR}/installToSrcInSrc/empty.cpp"
  COPYONLY
)
configure_file(
  "${RunCMake_SOURCE_DIR}/InstallPrefixInInterface.cmake"
  "${RunCMake_BINARY_DIR}/installToSrcInSrc/InstallPrefixInInterface.cmake"
  COPYONLY
)

set(RunCMake_TEST_OPTIONS "-DCMAKE_INSTALL_PREFIX=${RunCMake_BINARY_DIR}/installToSrcInSrc/InstallPrefixInInterface/prefix")
set(RunCMake_TEST_FILE "${RunCMake_BINARY_DIR}/installToSrcInSrc/InstallPrefixInInterface")
set(RunCMake_TEST_SOURCE_DIR "${RunCMake_BINARY_DIR}/installToSrcInSrc")
set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/installToSrcInSrc")
run_cmake(InstallToPrefixInSrcDirInSource)
unset(RunCMake_TEST_SOURCE_DIR)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_FILE)
unset(RunCMake_TEST_NO_CLEAN)
