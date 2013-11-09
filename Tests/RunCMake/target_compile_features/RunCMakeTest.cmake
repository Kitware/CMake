include(RunCMake)

run_cmake(not_enough_args)
run_cmake(alias_target)
run_cmake(utility_target)
run_cmake(invalid_args)
run_cmake(invalid_args_on_interface)
run_cmake(imported_target)
run_cmake(no_target)
run_cmake(not_a_cxx_feature)

string(REPLACE "-" ";" CMAKE_CXX_COMPILE_FEATURES "${CMAKE_CXX_COMPILE_FEATURES}")
if (";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";gnuxx_typeof;"
    OR ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";msvcxx_sealed;" )
  run_cmake(no_matching_cxx_feature)
endif()
