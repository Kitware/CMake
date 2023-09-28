include("${CMAKE_CURRENT_LIST_DIR}/../build-database-check.cmake")

check_build_database("export-build-database" "build_database.json" NO_EXIST)

if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  check_build_database("export-build-database" "build_database_CXX.json" JUST_CXX_MULTI)

  check_build_database("export-build-database" "build_database_CXX_Debug.json" CXX_AND_DEBUG)
  check_build_database("export-build-database" "build_database_Debug.json" JUST_DEBUG)
  check_build_database("export-build-database" "CMakeFiles/export_build_database.dir/Debug/CXX_build_database.json" JUST_TARGET_DEBUG)

  check_build_database("export-build-database" "build_database_CXX_Release.json" CXX_AND_RELEASE)
  check_build_database("export-build-database" "build_database_Release.json" JUST_RELEASE)
  check_build_database("export-build-database" "CMakeFiles/export_build_database.dir/Release/CXX_build_database.json" JUST_TARGET_RELEASE)
else ()
  check_build_database("export-build-database" "build_database_CXX.json" JUST_CXX)

  check_build_database("export-build-database" "CMakeFiles/export_build_database.dir/CXX_build_database.json" JUST_TARGET)
endif ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
