include(RunCMake)

run_cmake(ExperimentalWarning)

# Enable experimental feature and suppress warnings
set(RunCMake_TEST_OPTIONS
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES:STRING=e82e467b-f997-4464-8ace-b00808fff261"
  )

run_cmake(MissingTransitiveDependency)
run_cmake(MissingComponent)
run_cmake(MissingComponentDependency)
run_cmake(MissingTransitiveComponent)
run_cmake(MissingTransitiveComponentDependency)
