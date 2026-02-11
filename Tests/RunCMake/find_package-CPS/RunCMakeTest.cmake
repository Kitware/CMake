include(RunCMake)

function(run_cmake_build test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
  if(${ARGC} EQUAL 2)
    set(_build_type ${ARGV1})
  else()
    set(_build_type Release)
  endif()

  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=${_build_type})
  else()
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=${_build_type})
  endif()

  run_cmake(${test})

  set(RunCMake_TEST_NO_CLEAN TRUE)
  run_cmake_command(${test}-build ${CMAKE_COMMAND} --build . --config ${_build_type})
endfunction()


# General failure tests
run_cmake(InvalidCps1)
run_cmake(InvalidCps2)
run_cmake(InvalidCps3)
run_cmake(WrongName)
run_cmake(BadPrefix)

# Version-matching tests
run_cmake(ExactVersion)
run_cmake(CompatVersion)
run_cmake(MultipleVersions)
run_cmake(VersionLimit1)
run_cmake(VersionLimit2)
run_cmake(TransitiveVersion)
run_cmake(CustomVersion)

# Metadata Tests
run_cmake(License)

# Version-matching failure tests
run_cmake(MissingVersion1)
run_cmake(MissingVersion2)
run_cmake(VersionLimit3)
run_cmake(VersionLimit4)

# Component-related failure tests
run_cmake(MissingTransitiveDependency)
run_cmake(MissingComponent)
run_cmake(MissingComponentDependency)
run_cmake(MissingTransitiveComponentCPS)
run_cmake(MissingTransitiveComponentCMake)
run_cmake(MissingTransitiveComponentDependency)
run_cmake(SymbolicComponents)
