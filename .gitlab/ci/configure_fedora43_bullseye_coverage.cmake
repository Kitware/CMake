# Compile with Bullseye compiler wrappers, but do not test with them.
set(CMAKE_C_COMPILER "/opt/bullseye/bin/cc" CACHE PATH "")
set(CMAKE_CXX_COMPILER "/opt/bullseye/bin/c++" CACHE PATH "")

# Bullseye records a COVFILE id in object files, so they cannot be cached.
set(configure_no_sccache 1)

# Do not bootstrap for the coverage test suite.
set(CMAKE_SKIP_BOOTSTRAP_TEST TRUE CACHE BOOL "")

# Shrink stress tests when running with Bullseye.
set(ENV{KWSYS_TEST_PROCESS_1_COUNT} 11)

set(CMake_TEST_GUI "ON" CACHE BOOL "")
set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared,bmionly,build_database" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL_BAD "https://badtls-expired.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERSION "1.3" CACHE STRING "")
set(CMake_TEST_TLS_VERSION_URL_BAD "https://badtls-v1-1.kitware.com:8011" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora43_common.cmake")
