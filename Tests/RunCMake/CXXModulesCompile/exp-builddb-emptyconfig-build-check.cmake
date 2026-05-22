include("${CMAKE_CURRENT_LIST_DIR}/build-database-check.cmake")

check_build_database("exp-builddb-emptyconfig" "build_database.json" NO_EXIST)
check_build_database("exp-builddb-emptyconfig" "build_database_CXX.json" NO_EXIST)

check_build_database("exp-builddb-emptyconfig" "build_database_CXX_Debug.json" NO_EXIST)
check_build_database("exp-builddb-emptyconfig" "build_database_Debug.json" NO_EXIST)

check_build_database("exp-builddb-emptyconfig" "CMakeFiles/export_build_database.dir/CXX_build_database.json" JUST_TARGET)

string(REPLACE ";" "\n  " RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}")
