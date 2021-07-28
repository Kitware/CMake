cmake_minimum_required(VERSION 3.8)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/env_$ENV{CMAKE_CONFIGURATION}.cmake" OPTIONAL)

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

# We can now submit because we've configured. This is a cmb-superbuild-ism.
ctest_submit(PARTS Update)
ctest_submit(PARTS Configure)

if (configure_result)
  ctest_submit(PARTS Done)
  message(FATAL_ERROR
    "Failed to configure")
endif ()

include(ProcessorCount)
ProcessorCount(nproc)
if (NOT "$ENV{CTEST_MAX_PARALLELISM}" STREQUAL "")
  if (nproc GREATER "$ENV{CTEST_MAX_PARALLELISM}")
    set(nproc "$ENV{CTEST_MAX_PARALLELISM}")
  endif ()
endif ()

if (CTEST_CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  set(CTEST_BUILD_FLAGS "-j${nproc} -l${nproc}")
elseif (CTEST_CMAKE_GENERATOR MATCHES "Ninja")
  set(CTEST_BUILD_FLAGS "-l${nproc}")
endif ()

ctest_build(
  NUMBER_WARNINGS num_warnings
  RETURN_VALUE build_result)
ctest_submit(PARTS Build)

if (build_result)
  ctest_submit(PARTS Done)
  message(FATAL_ERROR
    "Failed to build")
endif ()

if ("$ENV{CTEST_NO_WARNINGS_ALLOWED}" AND num_warnings GREATER 0)
  ctest_submit(PARTS Done)
  message(FATAL_ERROR
    "Found ${num_warnings} warnings (treating as fatal).")
endif ()

set(ctest_label_args)
if (NOT "$ENV{CTEST_LABELS}" STREQUAL "")
  list(APPEND ctest_label_args
    INCLUDE_LABEL "$ENV{CTEST_LABELS}")
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/ctest_exclusions.cmake")
ctest_test(
  PARALLEL_LEVEL "${nproc}"
  TEST_LOAD "${nproc}"
  OUTPUT_JUNIT "${CTEST_BINARY_DIRECTORY}/junit.xml"
  RETURN_VALUE test_result
  ${ctest_label_args}
  EXCLUDE "${test_exclusions}")
ctest_submit(PARTS Test)

if (test_result)
  ctest_submit(PARTS Done)
  message(FATAL_ERROR
    "Failed to test")
endif ()

ctest_submit(PARTS Done)
