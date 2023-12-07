include("${CMAKE_CURRENT_LIST_DIR}/check-json.cmake")

if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(have_file 0)
  foreach (CXXModules_config IN ITEMS Release Debug RelWithDebInfo MinSizeRel)
    if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/ninja-exports-public.dir/${CXXModules_config}/CXXDependInfo.json")
      continue ()
    endif ()
    set(have_file 1)

    set(CMAKE_BUILD_TYPE "${CXXModules_config}")

    file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/ninja-exports-public.dir/${CXXModules_config}/CXXDependInfo.json" actual_contents)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/expect/NinjaDependInfoExport-public.json" expect_contents)
    check_json("${actual_contents}" "${expect_contents}")

    file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/ninja-exports-private.dir/${CXXModules_config}/CXXDependInfo.json" actual_contents)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/expect/NinjaDependInfoExport-private.json" expect_contents)
    check_json("${actual_contents}" "${expect_contents}")
  endforeach ()

  if (NOT have_file)
    list(APPEND RunCMake_TEST_FAILED
      "No recognized build configurations found.")
  endif ()
else ()
  set(CXXModules_config "${CXXModules_default_build_type}")
  set(CMAKE_BUILD_TYPE "${CXXModules_default_build_type}")

  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/ninja-exports-public.dir/CXXDependInfo.json" actual_contents)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/expect/NinjaDependInfoExport-public.json" expect_contents)
  check_json("${actual_contents}" "${expect_contents}")

  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/ninja-exports-private.dir/CXXDependInfo.json" actual_contents)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/expect/NinjaDependInfoExport-private.json" expect_contents)
  check_json("${actual_contents}" "${expect_contents}")
endif ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
