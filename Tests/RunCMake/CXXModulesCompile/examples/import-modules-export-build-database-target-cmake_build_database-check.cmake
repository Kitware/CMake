include("${CMAKE_CURRENT_LIST_DIR}/build-database-check.cmake")
set(item_filter "-ifcOnly")

if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
  check_build_database("export-build-database-imported" "build_database.json" ALL_MULTI)
  check_build_database("export-build-database-imported" "build_database_CXX.json" JUST_CXX_MULTI)

  check_build_database("export-build-database-imported" "build_database_CXX_Debug.json" CXX_AND_DEBUG)
  check_build_database("export-build-database-imported" "build_database_Debug.json" JUST_DEBUG)
  check_build_database("export-build-database-imported" "CMakeFiles/use_import_interfaces.dir/Debug/CXX_build_database.json" JUST_TARGET_DEBUG)

  check_build_database("export-build-database-imported" "build_database_CXX_Release.json" CXX_AND_RELEASE)
  check_build_database("export-build-database-imported" "build_database_Release.json" JUST_RELEASE)
  check_build_database("export-build-database-imported" "CMakeFiles/use_import_interfaces.dir/Release/CXX_build_database.json" JUST_TARGET_RELEASE)
else ()
  check_build_database("export-build-database-imported" "build_database.json" ALL)
  check_build_database("export-build-database-imported" "build_database_CXX.json" JUST_CXX)

  check_build_database("export-build-database-imported" "CMakeFiles/use_import_interfaces.dir/CXX_build_database.json" JUST_TARGET)
endif ()

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
