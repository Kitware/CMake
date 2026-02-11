file(GLOB synth_dirs
  "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/depchain_with_modules_json_file@synth_*.dir")

list(LENGTH synth_dirs synth_dirs_len)
if (NOT synth_dirs_len EQUAL 1)
  return()
endif()

list(GET synth_dirs 0 synth_dir)

if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(modules_json_path "${synth_dir}/Debug/CXXModules.json")
else ()
  set(modules_json_path "${synth_dir}/CXXModules.json")
endif ()

file(TOUCH_NOCREATE "${modules_json_path}")
