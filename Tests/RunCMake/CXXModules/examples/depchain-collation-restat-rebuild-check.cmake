if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(dep_collation_restat "CMakeFiles/depchain_collation_restat.dir/Debug/main.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")
  set(collation_restat "CMakeFiles/depchain_with_collation_restat.dir/Debug/CXXModules.json")
else ()
  set(dep_collation_restat "CMakeFiles/depchain_collation_restat.dir/main.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")
  set(collation_restat "CMakeFiles/depchain_with_collation_restat.dir/CXXModules.json")
endif ()

if (NOT "${RunCMake_TEST_BINARY_DIR}/${collation_restat}" IS_NEWER_THAN "${RunCMake_TEST_BINARY_DIR}/${dep_collation_restat}")
  list(APPEND RunCMake_TEST_FAILED
    "Object '${dep_collation_restat}' should not have recompiled if '${collation_restat}' did not change content.")
endif ()
