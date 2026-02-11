include(RunCMake)

# Test experimental gate
run_cmake(ExperimentalGate)
run_cmake(ExperimentalWarning)

# Test version check author warning
# TODO Move to be with other tests when experimental gate is removed.
run_cmake(VersionCheckWarning)

# Enable experimental feature and suppress warnings
set(RunCMake_TEST_OPTIONS
  -Wno-dev
  "-DCMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO:STRING=7fa7d13b-8308-4dc7-af39-9e450456d68f"
  "-DCMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES:STRING=e82e467b-f997-4464-8ace-b00808fff261"
  )

# Test incorrect usage
run_cmake(BadArgs0)
run_cmake(BadArgs1)
run_cmake(BadArgs2)
run_cmake(BadArgs3)
run_cmake(BadName)
run_cmake(DuplicateOutput)
run_cmake(BadDefaultTarget)
run_cmake(ReferencesNonExportedTarget)
run_cmake(ReferencesWronglyExportedTarget)
run_cmake(ReferencesWronglyImportedTarget)
run_cmake(ReferencesWronglyNamespacedTarget)
run_cmake(DependsMultipleDifferentNamespace)
run_cmake(DependsMultipleDifferentSets)
run_cmake(LinkInterfaceGeneratorExpression)
run_cmake(CompileOnlyRecursive)
run_cmake(LinkOnlyRecursive)

# Test functionality
run_cmake(Appendix)
run_cmake(InterfaceProperties)
run_cmake(Metadata)
run_cmake(ProjectMetadata)
run_cmake(NoProjectMetadata)
run_cmake(Minimal)
run_cmake(MinimalVersion)
run_cmake(LowerCaseFile)
run_cmake(Requirements)
run_cmake(LinkDependentLibraries)
run_cmake(ExportSymbolicComponent)
run_cmake(TargetTypes)
run_cmake(DependsMultiple)
run_cmake(CompileOnly)
run_cmake(LinkOnly)
run_cmake(Config)
run_cmake(EmptyConfig)
run_cmake(FileSetHeaders)
run_cmake(DependencyVersionCMake)
run_cmake(DependencyVersionCps)
run_cmake(TransitiveSymbolicComponent)
run_cmake(VersionCheck)
# run_cmake(VersionCheckWarning)
run_cmake(VersionCheckError)
