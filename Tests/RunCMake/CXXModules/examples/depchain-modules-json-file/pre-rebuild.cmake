if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(modules_json_path "CMakeFiles/depchain_with_modules_json_file.dir/Debug/CXXModules.json")
else ()
  set(modules_json_path "CMakeFiles/depchain_with_modules_json_file.dir/CXXModules.json")
endif ()

file(TOUCH_NOCREATE "${RunCMake_TEST_BINARY_DIR}/${modules_json_path}")
