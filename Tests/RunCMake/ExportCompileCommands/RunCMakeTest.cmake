include(RunCMake)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS -DCMAKE_CONFIGURATION_TYPES=Debug)
else()
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE=Debug)
endif()

run_cmake_with_options(BeforeProject -DCMAKE_PROJECT_INCLUDE_BEFORE=BeforeProjectBEFORE.cmake)
run_cmake(CustomCompileRule)
run_cmake(Properties)
run_cmake(PropertiesGenerateCommand)
