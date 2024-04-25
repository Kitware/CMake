if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_ISPC "ON" CACHE STRING "")
endif()
set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared,bmionly" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
