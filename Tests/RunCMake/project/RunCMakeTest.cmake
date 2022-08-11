include(RunCMake)

# Use an initial cache file to define the project() variables
# to avoid long command lines. Also see the CMakeOnly test case
# which tests some of the individual variables one at a time.
# Here, we are focused on testing that the variables are all injected
# at the expected points in the expected order.
run_cmake_with_options(CodeInjection
  -C "${CMAKE_CURRENT_LIST_DIR}/CodeInjection/initial_cache.cmake"
)

if(CMake_TEST_RESOURCES)
  run_cmake(ExplicitRC)
endif()
run_cmake(LanguagesImplicit)
run_cmake(LanguagesEmpty)
run_cmake(LanguagesNONE)
run_cmake(LanguagesTwice)
run_cmake(LanguagesUnordered)
if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  run_cmake(LanguagesUsedButNotEnabled)
endif()
run_cmake(ProjectDescription)
run_cmake(ProjectDescription2)
run_cmake(ProjectDescriptionNoArg)
run_cmake(ProjectDescriptionNoArg2)
run_cmake(ProjectHomepage)
run_cmake(ProjectHomepage2)
run_cmake(ProjectHomepageNoArg)
run_cmake(ProjectIsTopLevel)
run_cmake(ProjectIsTopLevelMultiple)
run_cmake(ProjectIsTopLevelSubdirectory)
run_cmake(ProjectTwice)
run_cmake(VersionAndLanguagesEmpty)
run_cmake(VersionEmpty)
run_cmake(VersionInvalid)
run_cmake(VersionMissingLanguages)
run_cmake(VersionMissingValueOkay)
run_cmake(VersionTwice)
run_cmake(VersionMax)

run_cmake(CMP0048-OLD)
run_cmake(CMP0048-OLD-VERSION)
run_cmake(CMP0048-WARN)
run_cmake(CMP0048-NEW)

run_cmake(CMP0096-WARN)
run_cmake(CMP0096-OLD)
run_cmake(CMP0096-NEW)
