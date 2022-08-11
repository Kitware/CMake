include(RunCMake)

run_cmake(ExplicitCMakeLists)
run_cmake(ImplicitCMakeLists)
run_cmake(InterfaceLibSources)
run_cmake_with_options(SearchPaths -DCMAKE_CONFIGURATION_TYPES=Debug)
run_cmake(InheritedParameters)

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
    set(maybe_destination -destination platform=macOS)
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

# Isolate device tests from host architecture selection.
unset(ENV{CMAKE_OSX_ARCHITECTURES})

# Use a single build tree for a few tests without cleaning.

if(NOT XCODE_VERSION VERSION_LESS 5)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeInstallIOS-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=iOS"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_BINARY_DIR}/ios_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeInstallIOS)
  run_cmake_command(XcodeInstallIOS-install ${CMAKE_COMMAND} --build . --target install)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeBundlesOSX-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=Darwin"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeBundles)
  run_cmake_command(XcodeBundles-build-macOS ${CMAKE_COMMAND} --build .)
  run_cmake_command(XcodeBundles-install-macOS ${CMAKE_COMMAND} --build . --target install)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeBundlesIOS-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=iOS"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeBundles)
  run_cmake_command(XcodeBundles-build-iOS ${CMAKE_COMMAND} --build .)
  run_cmake_command(XcodeBundles-install-iOS ${CMAKE_COMMAND} --build . --target install)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)
endif()

if(NOT XCODE_VERSION VERSION_LESS 7)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeBundlesWatchOS-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=watchOS"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeBundles)
  run_cmake_command(XcodeBundles-build-watchOS ${CMAKE_COMMAND} --build .)
  run_cmake_command(XcodeBundles-install-watchOS ${CMAKE_COMMAND} --build . --target install)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)
endif()

if(NOT XCODE_VERSION VERSION_LESS 7.1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeBundlesTvOS-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=tvOS"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeBundles)
  run_cmake_command(XcodeBundles-build-tvOS ${CMAKE_COMMAND} --build .)
  run_cmake_command(XcodeBundles-install-tvOS ${CMAKE_COMMAND} --build . --target install)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)
endif()

if(NOT XCODE_VERSION VERSION_LESS 7)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/osx.cmake")
  run_cmake(XcodeTbdStub)
  unset(RunCMake_TEST_OPTIONS)
endif()

if(XCODE_VERSION VERSION_GREATER_EQUAL 6)
  # XcodeIOSInstallCombined
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeIOSInstallCombined-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=iOS"
    "-DCMAKE_IOS_INSTALL_COMBINED=YES"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeIOSInstallCombined)
  run_cmake_command(XcodeIOSInstallCombined-build ${CMAKE_COMMAND} --build .)
  if(XCODE_VERSION VERSION_LESS 12)
    run_cmake_command(XcodeIOSInstallCombined-install ${CMAKE_COMMAND} --build . --target install)
  endif()
  # --build defaults to Debug, --install defaults to Release, so we have to
  # specify the configuration explicitly
  run_cmake_command(XcodeIOSInstallCombined-cmakeinstall
    ${CMAKE_COMMAND} --install . --config Debug
  )

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)

  # XcodeIOSInstallCombinedPrune
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeIOSInstallCombinedPrune-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=iOS"
    "-DCMAKE_IOS_INSTALL_COMBINED=YES"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeIOSInstallCombinedPrune)
  run_cmake_command(XcodeIOSInstallCombinedPrune-build ${CMAKE_COMMAND} --build .)
  if(XCODE_VERSION VERSION_LESS 12)
    run_cmake_command(XcodeIOSInstallCombinedPrune-install ${CMAKE_COMMAND} --build . --target install)
  endif()
  # --build defaults to Debug, --install defaults to Release, so we have to
  # specify the configuration explicitly
  run_cmake_command(XcodeIOSInstallCombinedPrune-cmakeinstall
    ${CMAKE_COMMAND} --install . --config Debug
  )

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)

  # XcodeIOSInstallCombinedSingleArch
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeIOSInstallCombinedSingleArch-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=iOS"
    "-DCMAKE_IOS_INSTALL_COMBINED=YES"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeIOSInstallCombinedSingleArch)
  run_cmake_command(XcodeIOSInstallCombinedSingleArch-build ${CMAKE_COMMAND} --build .)
  if(XCODE_VERSION VERSION_LESS 12)
    run_cmake_command(XcodeIOSInstallCombinedSingleArch-install ${CMAKE_COMMAND} --build . --target install)
  endif()
  # --build defaults to Debug, --install defaults to Release, so we have to
  # specify the configuration explicitly
  run_cmake_command(XcodeIOSInstallCombinedSingleArch-cmakeinstall
    ${CMAKE_COMMAND} --install . --config Debug
  )

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)
endif()

if(NOT XCODE_VERSION VERSION_LESS 5)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeMultiplatform-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "${IOS_DEPLOYMENT_TARGET}")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeMultiplatform)

  # build ios before macos
  run_cmake_command(XcodeMultiplatform-iphonesimulator-build ${CMAKE_COMMAND} --build . -- -sdk iphonesimulator)
  run_cmake_command(XcodeMultiplatform-iphonesimulator-install ${CMAKE_COMMAND} --build . --target install -- -sdk iphonesimulator DESTDIR=${RunCMake_TEST_BINARY_DIR}/_install_iphonesimulator)

  run_cmake_command(XcodeMultiplatform-macosx-build ${CMAKE_COMMAND} --build . -- -sdk macosx)
  run_cmake_command(XcodeMultiplatform-macosx-install ${CMAKE_COMMAND} --build . --target install -- -sdk macosx DESTDIR=${RunCMake_TEST_BINARY_DIR}/_install_macosx)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_OPTIONS)
endif()

if(XCODE_VERSION VERSION_GREATER_EQUAL 8)
  function(deployment_target_test SystemName SDK)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/DeploymentTarget-${SDK}-build)
    set(RunCMake_TEST_NO_CLEAN 1)
    set(RunCMake_TEST_OPTIONS "-DCMAKE_SYSTEM_NAME=${SystemName}" "-DCMAKE_OSX_SYSROOT=${SDK}")

    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

    run_cmake(DeploymentTarget)
    run_cmake_command(DeploymentTarget-${SDK} ${CMAKE_COMMAND} --build .)
  endfunction()

  deployment_target_test(Darwin macosx)
  deployment_target_test(iOS iphoneos)
  deployment_target_test(iOS iphonesimulator)
  deployment_target_test(tvOS appletvos)
  deployment_target_test(tvOS appletvsimulator)
  deployment_target_test(watchOS watchos)
  deployment_target_test(watchOS watchsimulator)
endif()

if(XCODE_VERSION VERSION_GREATER_EQUAL 8)
  function(xctest_lookup_test SystemName SDK)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XCTestLookup-${SDK}-build)
    set(RunCMake_TEST_OPTIONS "-DCMAKE_SYSTEM_NAME=${SystemName}" "-DCMAKE_OSX_SYSROOT=${SDK}")

    run_cmake(XCTestLookup)
  endfunction()

  xctest_lookup_test(Darwin macosx)
  xctest_lookup_test(iOS iphoneos)
  xctest_lookup_test(iOS iphonesimulator)
  xctest_lookup_test(tvOS appletvos)
  xctest_lookup_test(tvOS appletvsimulator)
endif()

if(XCODE_VERSION VERSION_GREATER_EQUAL 8)
  function(XcodeRemoveExcessiveISystemSDK SDK)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeRemoveExcessiveISystemSDK-${SDK}-build)
    set(RunCMake_TEST_OPTIONS "-DCMAKE_SYSTEM_NAME=iOS" "-DCMAKE_OSX_SYSROOT=${SDK}")
    run_cmake(XcodeRemoveExcessiveISystem)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(XcodeRemoveExcessiveISystemSDK-${SDK}-build ${CMAKE_COMMAND} --build .)
  endfunction()

  XcodeRemoveExcessiveISystemSDK(iphoneos)
  XcodeRemoveExcessiveISystemSDK(iphonesimulator)
endif()

if (XCODE_VERSION VERSION_GREATER_EQUAL 7.3)
  function(xctest_add_bundle_test SystemName SDK BuildSystemVersion ExpectedOutputDir)
    set(RunCMake_TEST_BINARY_DIR
      ${RunCMake_BINARY_DIR}/DeploymentTarget-${SystemName}-${SDK}-${BuildSystemVersion}-build)
    set(RunCMake_TEST_OPTIONS
      "-DCMAKE_SYSTEM_NAME=${SystemName}"
      "-DCMAKE_OSX_SYSROOT=${SDK}"
      "-DTEST_EXPECTED_OUTPUT_DIR=${ExpectedOutputDir}")
    unset(RunCMake_GENERATOR_TOOLSET)
    if(BuildSystemVersion)
      set(RunCMake_GENERATOR_TOOLSET "buildsystem=${BuildSystemVersion}")
    endif()
    run_cmake(XCTestAddBundle)
  endfunction()

  if(XCODE_VERSION VERSION_GREATER_EQUAL 12)
    xctest_add_bundle_test(Darwin macosx "1" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>/PlugIns")
    xctest_add_bundle_test(Darwin macosx "12" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>/PlugIns")
    xctest_add_bundle_test(iOS iphonesimulator "1" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>/PlugIns")
    if (XCODE_VERSION VERSION_LESS 12.5)
      xctest_add_bundle_test(iOS iphonesimulator "12" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>")
    else()
      xctest_add_bundle_test(iOS iphonesimulator "12" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>/PlugIns")
    endif()
  else()
    xctest_add_bundle_test(Darwin macosx "" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>/PlugIns")
    xctest_add_bundle_test(iOS iphonesimulator "" "$<TARGET_BUNDLE_CONTENT_DIR:TestedApp>/PlugIns")
  endif()
endif()

# Please add macOS-only tests above before the device-specific tests.
