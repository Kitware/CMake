if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(dep_modules_json_path "CMakeFiles/depchain_modules_json_file.dir/Debug/CXX.dd")
  set(modules_json_path "CMakeFiles/depchain_with_modules_json_file.dir/Debug/CXXModules.json")
else ()
  set(dep_modules_json_path "CMakeFiles/depchain_modules_json_file.dir/CXX.dd")
  set(modules_json_path "CMakeFiles/depchain_with_modules_json_file.dir/CXXModules.json")
endif ()

if ("${RunCMake_TEST_BINARY_DIR}/${modules_json_path}" IS_NEWER_THAN "${RunCMake_TEST_BINARY_DIR}/${dep_modules_json_path}")
  list(APPEND RunCMake_TEST_FAILED
    "Object '${dep_modules_json_path}' should have recompiled if '${modules_json_path}' changed.")
endif ()
