if (NOT DEFINED "ENV{GITLAB_CI}")
  message(FATAL_ERROR
    "This script assumes it is being run inside of GitLab-CI")
endif ()

# Set up the source and build paths.
set(CTEST_SOURCE_DIRECTORY "$ENV{CI_PROJECT_DIR}")
if("$ENV{CMAKE_CI_IN_SYMLINK_TREE}")
  set(CTEST_SOURCE_DIRECTORY "$ENV{CI_PROJECT_DIR}/work/cmake")
endif()
if("$ENV{CMAKE_CI_INPLACE}")
  set(CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}")
elseif("$ENV{CMAKE_CI_IN_SYMLINK_TREE}")
  set(CTEST_BINARY_DIRECTORY "$ENV{CI_PROJECT_DIR}/work/build")
else()
  set(CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/build")
endif()
if (NOT "$ENV{CTEST_SOURCE_SUBDIRECTORY}" STREQUAL "")
  string(APPEND CTEST_SOURCE_DIRECTORY "/$ENV{CTEST_SOURCE_SUBDIRECTORY}")
endif ()

if ("$ENV{CMAKE_CONFIGURATION}" STREQUAL "")
  message(FATAL_ERROR
    "The CMAKE_CONFIGURATION environment variable is required to know what "
    "cache initialization file to use.")
endif ()

# Set the build metadata.
if(NOT "$ENV{CMAKE_CI_BUILD_NAME}" STREQUAL "")
  set(CTEST_BUILD_NAME "$ENV{CI_PROJECT_NAME}-$ENV{CMAKE_CI_BUILD_NAME}")
else()
  set(CTEST_BUILD_NAME "$ENV{CI_PROJECT_NAME}-$ENV{CMAKE_CONFIGURATION}")
endif()
set(CTEST_SITE "gitlab-ci")
set(ctest_model "Experimental")

# Default to Release builds.
if (NOT "$ENV{CMAKE_CI_BUILD_TYPE}" STREQUAL "")
  set(CTEST_BUILD_CONFIGURATION "$ENV{CMAKE_CI_BUILD_TYPE}")
endif ()
if (NOT CTEST_BUILD_CONFIGURATION)
  set(CTEST_BUILD_CONFIGURATION "Release")
endif ()
set(CTEST_CONFIGURATION_TYPE "${CTEST_BUILD_CONFIGURATION}")

# Default to using Ninja.
if (NOT "$ENV{CMAKE_GENERATOR}" STREQUAL "")
  set(CTEST_CMAKE_GENERATOR "$ENV{CMAKE_GENERATOR}")
endif ()
if (NOT CTEST_CMAKE_GENERATOR)
  set(CTEST_CMAKE_GENERATOR "Ninja")
endif ()

# Set the toolset and platform if requested.
if (NOT "$ENV{CMAKE_GENERATOR_PLATFORM}" STREQUAL "")
  set(CTEST_CMAKE_GENERATOR_PLATFORM "$ENV{CMAKE_GENERATOR_PLATFORM}")
endif ()
if (NOT "$ENV{CMAKE_GENERATOR_TOOLSET}" STREQUAL "")
  set(CTEST_CMAKE_GENERATOR_TOOLSET "$ENV{CMAKE_GENERATOR_TOOLSET}")
endif ()

# Determine the group to submit to.
set(ctest_group "Experimental")
if (NOT "$ENV{CI_MERGE_REQUEST_ID}" STREQUAL "")
  set(ctest_group "merge-requests")
elseif (NOT "$ENV{CMAKE_CI_PROJECT_CONTINUOUS_BRANCH}" STREQUAL "" AND "$ENV{CMAKE_CI_PROJECT_CONTINUOUS_BRANCH}" STREQUAL "$ENV{CI_COMMIT_BRANCH}" AND NOT "$ENV{CMAKE_CI_JOB_CONTINUOUS}" STREQUAL "")
  set(ctest_model "Continuous")
  if (NOT "$ENV{CMAKE_CI_JOB_HELP}" STREQUAL "")
    set(ctest_group "Continuous Help")
  else()
    set(ctest_group "Continuous")
  endif()
  string(PREPEND CTEST_BUILD_NAME "continuous-")
elseif (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(ctest_model "Nightly")
  set(ctest_group "Nightly Expected")
  string(PREPEND CTEST_BUILD_NAME "nightly-")
elseif ("$ENV{CI_PROJECT_PATH}" STREQUAL "cmake/cmake")
  if ("$ENV{CI_COMMIT_REF_NAME}" STREQUAL "master")
    set(ctest_group "master")
  elseif ("$ENV{CI_COMMIT_REF_NAME}" STREQUAL "release")
    set(ctest_group "release")
  endif ()
endif ()
