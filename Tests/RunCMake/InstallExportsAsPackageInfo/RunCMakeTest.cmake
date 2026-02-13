include(RunCMake)

# Test experimental gate
run_cmake(ExperimentalGate)
run_cmake(ExperimentalWarning)

# Enable experimental feature and suppress warnings
set(RunCMake_TEST_OPTIONS
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_MAPPED_PACKAGE_INFO:STRING=ababa1b5-7099-495f-a9cd-e22d38f274f2"
  )

# Test incorrect usage
run_cmake(MissingPackageName)
run_cmake(MissingAppendixName)
run_cmake(MissingExport)
run_cmake(BadDirective)

# Test functionality
run_cmake(SampleExport)
run_cmake(LowerCase)
