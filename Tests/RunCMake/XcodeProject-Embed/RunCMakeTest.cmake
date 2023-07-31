include(RunCMake)

# Build dependencies that the other tests will use and treat as external.
# Always build in the Debug configuration so that the path to the framework
# is predictable.
function(ExternalDependencies)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ExternalDependencies-build)
  set(externalFramework ${RunCMake_TEST_BINARY_DIR}/Debug/sharedFrameworkExt.framework PARENT_SCOPE)
  set(externalDylib ${RunCMake_TEST_BINARY_DIR}/Debug/libsharedDylibExt.dylib PARENT_SCOPE)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(ExternalDependencies)
  run_cmake_command(ExternalDependencies-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target sharedFrameworkExt sharedDylibExt
  )
endfunction()
ExternalDependencies()

function(TestFlagsOn testName dependencyName)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-${dependencyName}-build)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${testName})
  run_cmake_command(${testName}-${dependencyName}-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target app
  )
endfunction()

foreach(dependency ${externalFramework} ${externalDylib})
  cmake_path(GET dependency FILENAME dependencyName)
  set(RunCMake_TEST_OPTIONS -DEXTERNAL_DEPENDENCY=${dependency} -DEXTERNAL_DEPENDENCY_NAME=${dependencyName})
  run_cmake(EmbedFrameworksFlagsOff)
  TestFlagsOn(EmbedFrameworksFlagsOnNoSubdir ${dependencyName})
  TestFlagsOn(EmbedFrameworksFlagsOnWithSubdir ${dependencyName})
endforeach()
unset(RunCMake_TEST_OPTIONS)

function(TestAppExtension platform)
  set(testName EmbedAppExtensions-${platform})
  if(NOT platform STREQUAL "macOS")
    set(RunCMake_TEST_OPTIONS -DCMAKE_SYSTEM_NAME=${platform})
  endif()
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${testName})
  run_cmake_command(${testName}-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target app
  )
endfunction()

function(TestExtensionKitExtension platform)
  set(testName EmbedExtensionKitExtensions-${platform})
  if(NOT platform STREQUAL "macOS")
    set(RunCMake_TEST_OPTIONS -DCMAKE_SYSTEM_NAME=${platform})
  endif()
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${testName})
  run_cmake_command(${testName}-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target app
  )
endfunction()

function(TestEmbedCommon what platform)
  set(testName Embed${what}-${platform})
  if(NOT platform STREQUAL "macOS")
    set(RunCMake_TEST_OPTIONS -DCMAKE_SYSTEM_NAME=${platform})
  endif()
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${testName}-build)

  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  run_cmake(${testName})
  run_cmake_command(${testName}-build
    ${CMAKE_COMMAND} --build ${RunCMake_TEST_BINARY_DIR}
                     --config Debug
                     --target app
  )
endfunction()

# Isolate device tests from host architecture selection.
unset(ENV{CMAKE_OSX_ARCHITECTURES})

if(XCODE_VERSION VERSION_GREATER_EQUAL 8)
  # The various flag on/off combinations are tested by the EmbedFrameworks...
  # tests, so we don't duplicate all the combinations here. We only verify the
  # defaults, which is to remove headers on copy, but not code sign.
  TestAppExtension(macOS)
  TestAppExtension(iOS)
endif()

if(XCODE_VERSION VERSION_GREATER_EQUAL 14.1)
  # The various flag on/off combinations are tested by the EmbedFrameworks...
  # tests, so we don't duplicate all the combinations here. We only verify the
  # defaults, which is to remove headers on copy, but not code sign.
  TestAppExtension(macOS)
  TestAppExtension(iOS)
  TestEmbedCommon(Resources macOS)
  TestEmbedCommon(Resources iOS)
  TestEmbedCommon(PlugIns macOS)
endif()
