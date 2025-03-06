include(RunCMake)

run_cmake(ExperimentalWarning)

# Enable experimental feature and suppress warnings
set(RunCMake_TEST_OPTIONS
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES:STRING=e82e467b-f997-4464-8ace-b00808fff261"
  )

# Version-matching tests
run_cmake(ExactVersion)
run_cmake(CompatVersion)
run_cmake(MultipleVersions)
run_cmake(VersionLimit1)
run_cmake(VersionLimit2)
run_cmake(TransitiveVersion)
run_cmake(CustomVersion)

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
