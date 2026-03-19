set(CMake_TEST_CXX_STDLIB_MODULES_JSON "$ENV{CMAKE_CI_CXX_STDLIB_MODULES_JSON}" CACHE FILEPATH "")
set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared,bmionly,build_database,import_std23" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
