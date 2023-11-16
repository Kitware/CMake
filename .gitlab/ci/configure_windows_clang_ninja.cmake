if("$ENV{CMAKE_CI_BUILD_NAME}" MATCHES "(^|_)gnu(_|$)")
  set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared,bmionly" CACHE STRING "")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_clang_common.cmake")
