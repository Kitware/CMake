include(RunCMake)

# Test experimental gate
run_cmake(ExperimentalGate)
run_cmake(ExperimentalWarning)

# Enable experimental feature and suppress warnings
set(RunCMake_TEST_OPTIONS
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO:STRING=b80be207-778e-46ba-8080-b23bba22639e"
  )

# Test incorrect usage
run_cmake(BadArgs1)
run_cmake(BadArgs2)
run_cmake(BadArgs3)
run_cmake(BadArgs4)
run_cmake(BadArgs5)
run_cmake(BadDefaultTarget)
run_cmake(ReferencesNonExportedTarget)
run_cmake(ReferencesWronglyExportedTarget)
run_cmake(ReferencesWronglyImportedTarget)
run_cmake(ReferencesWronglyNamespacedTarget)

# Test functionality
run_cmake(Appendix)
run_cmake(InterfaceProperties)
run_cmake(Metadata)
run_cmake(Minimal)
run_cmake(LowerCaseFile)
run_cmake(Requirements)
run_cmake(TargetTypes)
