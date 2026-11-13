set(CMake_TEST_GUI "ON" CACHE BOOL "")
set(CMake_TEST_MODULE_COMPILATION "named,compile_commands,collation,partitions,internal_partitions,export_bmi,install_bmi,shared,bmionly,build_database,import_std23" CACHE STRING "")

# Do not bootstrap: BootstrapTest would record the listfiles of CMake's own build.
set(CMake_TEST_BOOTSTRAP FALSE CACHE BOOL "")

# Turn on the module tests of the Makefiles job so the report covers the
# modules, such as `Modules/Find*.cmake`, that only those tests run.
include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora45_makefiles_common.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora45_common.cmake")
