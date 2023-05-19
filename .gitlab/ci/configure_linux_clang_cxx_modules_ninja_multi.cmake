set(CMake_TEST_MODULE_COMPILATION "named,collation,partitions,internal_partitions,export_bmi,install_bmi,shared" CACHE STRING "")
set(CMake_TEST_MODULE_COMPILATION_RULES "${CMAKE_CURRENT_LIST_DIR}/cxx_modules_rules_clang.cmake" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
