include(RunCMake)

run_cmake(cache_properties)
run_cmake(directory_properties)
run_cmake(get_directory_property_empty)
run_cmake(get_directory_property_missing)
run_cmake(get_directory_property_missingWithDir)
run_cmake(global_properties)
run_cmake(install_properties)
run_cmake(source_properties)
run_cmake(source_properties_failures)
run_cmake(target_properties)
run_cmake(test_properties)
run_cmake(DebugConfigurations)

run_cmake(MissingArgument)
run_cmake(GlobalName)
run_cmake(BadTest)
run_cmake(BadTarget)
run_cmake(BadScope)
run_cmake(BadDirectory)
run_cmake(BadArgument)
run_cmake(VariableName)
run_cmake(NoTest)
run_cmake(NoTarget)
run_cmake(NoSource)
run_cmake(NoProperty)
run_cmake(NoCache)

# Since we are testing the GENERATOR_IS_MULTI_CONFIG property itself,
# don't rely on RunCMake_GENERATOR_IS_MULTI_CONFIG being set correctly
# and instead explicitly check for a match against those generators we
# expect to be multi-config
if(RunCMake_GENERATOR MATCHES "Visual Studio|Xcode|Ninja Multi-Config")
  run_cmake(IsMultiConfig)
else()
  run_cmake(NotMultiConfig)
endif()
