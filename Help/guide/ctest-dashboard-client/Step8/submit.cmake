cmake_minimum_required(VERSION 4.4)

set(CTEST_BUILD_NAME "Step8: presets")
site_name(CTEST_SITE)
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/../Project")
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")

# Note that we aren't setting CTEST_CMAKE_GENERATOR in this script any more.
# It is now defined in the configure preset instead.

set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

set(CTEST_UPDATE_COMMAND "git")
set(CTEST_UPDATE_VERSION_ONLY 1)

set(CTEST_USE_LAUNCHERS 1)
set(CTEST_COVERAGE_COMMAND "gcov")

# Select the preset, and the presets file that defines it, for the
# ctest_configure(), ctest_build(), and ctest_test() commands below.
set(CTEST_PRESET "coverage")
set(CTEST_PRESETS_FILE "${CTEST_SCRIPT_DIRECTORY}/CMakePresets.json")

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

ctest_start(Experimental)
ctest_update()
ctest_configure()
ctest_build()
ctest_test()
ctest_coverage()
ctest_submit()
