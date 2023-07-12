include(RunCMake)

# Isolate device tests from host architecture selection.
unset(ENV{CMAKE_OSX_ARCHITECTURES})

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

if(NOT XCODE_VERSION VERSION_LESS 15)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/XcodeBundlesVisionOS-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS
    "-DCMAKE_SYSTEM_NAME=visionOS"
    "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/_install")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(XcodeBundles)
  run_cmake_command(XcodeBundles-build-visionOS ${CMAKE_COMMAND} --build .)
  run_cmake_command(XcodeBundles-install-visionOS ${CMAKE_COMMAND} --build . --target install)

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
  # FIXME(#24011): Xcode 14 removed support for older architectures the test needs.
  if(XCODE_VERSION VERSION_LESS 14)
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
  endif()

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
  # XcodeMultiplatform
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

  # EffectivePlatformNameOFF
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/EffectivePlatformNameOFF-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_SYSTEM_NAME=iOS" "-DCMAKE_OSX_SYSROOT=iphonesimulator")

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(EffectivePlatformNameOFF)

  run_cmake_command(EffectivePlatformNameOFF-iphonesimulator-build ${CMAKE_COMMAND} --build .)
  run_cmake_command(EffectivePlatformNameOFF-iphonesimulator-install ${CMAKE_COMMAND} --build . --target install -- DESTDIR=${RunCMake_TEST_BINARY_DIR}/_install_iphonesimulator)

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
  if(XCODE_VERSION VERSION_GREATER_EQUAL 15)
    deployment_target_test(visionOS xros)
    deployment_target_test(visionOS xrsimulator)
  endif()
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
