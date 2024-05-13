if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(modmap_path "CMakeFiles/depchain_modmap.dir/Debug/main.cxx${CMAKE_CXX_OUTPUT_EXTENSION}.modmap")
else ()
  set(modmap_path "CMakeFiles/depchain_modmap.dir/main.cxx${CMAKE_CXX_OUTPUT_EXTENSION}.modmap")
endif ()

file(TOUCH_NOCREATE "${RunCMake_TEST_BINARY_DIR}/${modmap_path}")
