if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(object_path "CMakeFiles/depchain_modmap.dir/Debug/main.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")
else ()
  set(object_path "CMakeFiles/depchain_modmap.dir/main.cxx${CMAKE_CXX_OUTPUT_EXTENSION}")
endif ()
set(modmap_path "${object_path}.modmap")

if ("${RunCMake_TEST_BINARY_DIR}/${modmap_path}" IS_NEWER_THAN "${RunCMake_TEST_BINARY_DIR}/${object_path}")
  list(APPEND RunCMake_TEST_FAILED
    "Object '${object_path}' should have recompiled if '${modmap_path}' changed.")
endif ()
