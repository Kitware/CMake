cmake_minimum_required(VERSION 3.8)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

set(cmake_args
  -C "${CMAKE_CURRENT_LIST_DIR}/configure_$ENV{CMAKE_CONFIGURATION}.cmake")

# Create an entry in CDash.
ctest_start("${ctest_model}" GROUP "${ctest_group}")

# Gather update information.
find_package(Git)
set(CTEST_UPDATE_VERSION_ONLY ON)
set(CTEST_UPDATE_COMMAND "${GIT_EXECUTABLE}")
ctest_update()

# Configure the project.
ctest_configure(
  OPTIONS "${cmake_args}"
  RETURN_VALUE configure_result)

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

# We can now submit because we've configured.
ctest_submit(PARTS Update
  BUILD_ID build_id)
ctest_submit(PARTS Configure)

include("${CMAKE_CURRENT_LIST_DIR}/ctest_annotation.cmake")
ctest_annotation_report("${CTEST_BINARY_DIRECTORY}/annotations.json"
  "Build Summary" "https://open.cdash.org/build/${build_id}"
  "Update"        "https://open.cdash.org/build/${build_id}/update"
  "Configure"     "https://open.cdash.org/build/${build_id}/configure")
store_build_id("${build_id}")

if (configure_result)
  message(FATAL_ERROR
    "Failed to configure")
endif ()
