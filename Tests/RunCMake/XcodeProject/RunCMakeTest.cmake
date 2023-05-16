include(RunCMake)

function(RunClean)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Clean-build)
  run_cmake(Clean -DCMAKE_CONFIGURATION_TYPES=Debug)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(Clean-build xcodebuild)
  run_cmake_command(Clean-clean xcodebuild clean)
endfunction()
RunClean()

run_cmake(ExplicitCMakeLists)
run_cmake(ImplicitCMakeLists)
run_cmake(InterfaceLibSources)
run_cmake_with_options(SearchPaths -DCMAKE_CONFIGURATION_TYPES=Debug)
run_cmake_with_options(InheritedParameters -DCMake_TEST_Swift=${CMake_TEST_Swift})

run_cmake(XcodeFileType)
run_cmake(XcodeAttributeLocation)
run_cmake(XcodeAttributeGenex)
run_cmake(XcodeAttributeGenexError)
run_cmake(XcodeGenerateTopLevelProjectOnly)

if(XCODE_VERSION VERSION_GREATER_EQUAL 12)
  run_cmake(XcodeDuplicateCustomCommand)
endif()

function(XcodeGenerateTopLevelProjectOnlyWithObjectLibrary)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeGenerateTopLevelProjectOnlyWithObjectLibrary-build)
  run_cmake(XcodeGenerateTopLevelProjectOnlyWithObjectLibrary)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(XcodeGenerateTopLevelProjectOnlyWithObjectLibrary-build ${CMAKE_COMMAND} --build . --target shared_lib)
endfunction()

XcodeGenerateTopLevelProjectOnlyWithObjectLibrary()

function(LinkBinariesBuildPhase mode)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/LinkBinariesBuildPhase_${mode}-build)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_XCODE_LINK_BUILD_PHASE_MODE=${mode}")
  run_cmake(LinkBinariesBuildPhase_${mode})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(LinkBinariesBuildPhase_${mode}-build ${CMAKE_COMMAND} --build .)
endfunction()

LinkBinariesBuildPhase(NONE)
LinkBinariesBuildPhase(BUILT_ONLY)
LinkBinariesBuildPhase(KNOWN_LOCATION)
run_cmake(LinkBinariesBuildPhase_INVALID)

run_cmake(XcodeObjectNeedsEscape)
run_cmake(XcodeObjectNeedsQuote)
run_cmake(XcodeOptimizationFlags)
run_cmake(XcodePreserveNonOptimizationFlags)
run_cmake(XcodePreserveObjcFlag)
run_cmake(XcodePrecompileHeaders)
if (NOT XCODE_VERSION VERSION_LESS 6)
  run_cmake(XcodePlatformFrameworks)
endif()

run_cmake(PerConfigPerSourceFlags)
run_cmake(PerConfigPerSourceOptions)
run_cmake(PerConfigPerSourceDefinitions)
run_cmake(PerConfigPerSourceIncludeDirs)

if(XCODE_VERSION VERSION_GREATER_EQUAL 12)

  function(XcodeObjectLibsInTwoProjectsiOS)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeObjectLibsInTwoProjects-build-ios)
    set(RunCMake_TEST_OPTIONS "-DCMAKE_SYSTEM_NAME=iOS")

    run_cmake(XcodeObjectLibsInTwoProjects)

    set(RunCMake_TEST_NO_CLEAN 1)

    run_cmake_command(XcodeObjectLibsInTwoProjects-build-ios ${CMAKE_COMMAND} --build . --target shared_lib -- -sdk iphonesimulator)
  endfunction()

  XcodeObjectLibsInTwoProjectsiOS()

  function(XcodeObjectLibsInTwoProjectsMacOS)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeObjectLibsInTwoProjects-build-macos)

    run_cmake(XcodeObjectLibsInTwoProjects)

    set(RunCMake_TEST_NO_CLEAN 1)

    run_cmake_command(XcodeObjectLibsInTwoProjects-build-macos ${CMAKE_COMMAND} --build . --target shared_lib)
  endfunction()

  XcodeObjectLibsInTwoProjectsMacOS()

endif()

function(XcodeSchemaGeneration)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeSchemaGeneration-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_XCODE_GENERATE_SCHEME=ON")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeSchemaGeneration)
  if (XCODE_VERSION VERSION_GREATER_EQUAL 13)
    set(maybe_destination -destination platform=macOS,arch=${CMAKE_HOST_SYSTEM_PROCESSOR})
  else()
    set(maybe_destination "")
  endif()
  run_cmake_command(XcodeSchemaGeneration-build xcodebuild -scheme foo ${maybe_destination} build)
endfunction()

if(NOT XCODE_VERSION VERSION_LESS 7)
  XcodeSchemaGeneration()
  run_cmake(XcodeSchemaProperty)
endif()

function(XcodeDependOnZeroCheck)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeDependOnZeroCheck-build)
  set(RunCMake_TEST_NO_CLEAN 1)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeDependOnZeroCheck)
  run_cmake_command(XcodeDependOnZeroCheck-build ${CMAKE_COMMAND} --build . --target parentdirlib)
  run_cmake_command(XcodeDependOnZeroCheck-build ${CMAKE_COMMAND} --build . --target subdirlib)
endfunction()

XcodeDependOnZeroCheck()

function(XcodeObjcxxFlags testName)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)
  set(RunCMake_TEST_NO_CLEAN 1)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${testName})
  run_cmake_command(${testName}-build ${CMAKE_COMMAND} --build .)
endfunction()

XcodeObjcxxFlags(XcodeObjcFlags)
XcodeObjcxxFlags(XcodeObjcxxFlags)

function(XcodeRemoveExcessiveISystem)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeRemoveExcessiveISystem-build)
  run_cmake(XcodeRemoveExcessiveISystem)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(XcodeRemoveExcessiveISystem-build ${CMAKE_COMMAND} --build .)
endfunction()

XcodeRemoveExcessiveISystem()

function(XcodeXCConfig)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeXCConfig-build)
  run_cmake(XcodeXCConfig)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(XcodeXCConfig-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(XcodeXCConfig-build ${CMAKE_COMMAND} --build . --config Release)
endfunction()

XcodeXCConfig()

function(BundleLinkBundle)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/BundleLinkBundle-build)
  run_cmake(BundleLinkBundle)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(BundleLinkBundle-build ${CMAKE_COMMAND} --build .)
endfunction()

BundleLinkBundle()

# Please add device-specific tests to '../XcodeProject-Device/RunCMakeTest.cmake'.
