file(GLOB synth_dirs
  "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/depchain_with_modules_json_file@synth_*.dir")

list(LENGTH synth_dirs synth_dirs_len)
if (NOT synth_dirs_len EQUAL 1)
  list(APPEND RunCMake_TEST_FAILED
    "Expected exactly one synthetic target for consuming 'depchain_with_modules_json_file' but found ${synth_dirs_len}: ${synth_dirs}")
endif ()

list(GET synth_dirs 0 synth_dir)

if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(dep_modules_json_path "CMakeFiles/depchain_modules_json_file.dir/Debug/CXX.dd")
  set(modules_json_path "${synth_dir}/Debug/CXXModules.json")
else ()
  set(dep_modules_json_path "CMakeFiles/depchain_modules_json_file.dir/CXX.dd")
  set(modules_json_path "${synth_dir}/CXXModules.json")
endif ()


if ("${modules_json_path}" IS_NEWER_THAN "${RunCMake_TEST_BINARY_DIR}/${dep_modules_json_path}")
  cmake_path(RELATIVE_PATH modules_json_path
    BASE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  list(APPEND RunCMake_TEST_FAILED
    "Object '${dep_modules_json_path}' should have recompiled if '${modules_json_path}' changed.")
endif ()
