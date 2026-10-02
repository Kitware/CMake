
include(RunCMake)

run_cmake(no-arguments)
run_cmake(list-conversion)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  run_cmake_with_options(simple -DCMAKE_CONFIGURATION_TYPES=Debug)
else()
  run_cmake_with_options(simple -DCMAKE_BUILD_TYPE=Debug)
endif()
