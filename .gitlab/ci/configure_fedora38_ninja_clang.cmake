set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared" CACHE STRING "")
set(CMake_TEST_MODULE_COMPILATION_RULES "${CMAKE_CURRENT_LIST_DIR}/cxx_modules_rules_clang.cmake" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora38_common_clang.cmake")
