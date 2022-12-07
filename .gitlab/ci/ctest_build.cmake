cmake_minimum_required(VERSION 3.8)

include("${CMAKE_CURRENT_LIST_DIR}/gitlab_ci.cmake")

# Read the files from the build directory.
ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")

# Pick up from where the configure left off.
ctest_start(APPEND)

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

set(ctest_build_args)

# IWYU debugging:
# - set the name of the target (if not one of the main libraries)
# - set the name of the source (without extension, relative to `Source/`)
# - uncomment the debugging support in the relevant `configure_*_iwyu.cmake` file
set(iwyu_source_target) # set for "other" targets not the "main" 3 libraries
set(iwyu_source_name) # e.g., cmTarget
if (iwyu_source_name AND "$ENV{CMAKE_CONFIGURATION}" MATCHES "iwyu")
  if (NOT iwyu_source_target)
    if (iwyu_source_name MATCHES "^(CTest/|cmCTest$)")
      set(iwyu_source_target "CTestLib")
    elseif (iwyu_source_name MATCHES "^CPack/")
      set(iwyu_source_target "CPackLib")
    else () # Assume everything else is in CMakeLib
      set(iwyu_source_target "CMakeLib")
    endif ()
  endif ()
  list(APPEND ctest_build_args
    TARGET "Source/CMakeFiles/${iwyu_source_target}.dir/${iwyu_source_name}.cxx.o")
endif ()

ctest_build(
  NUMBER_ERRORS num_errors
  NUMBER_WARNINGS num_warnings
  RETURN_VALUE build_result
  ${ctest_build_args})
ctest_submit(PARTS Build)

include("${CMAKE_CURRENT_LIST_DIR}/ctest_annotation.cmake")
ctest_annotation_report("${CTEST_BINARY_DIRECTORY}/annotations.json"
  "Build Errors (${num_errors})"      "https://open.cdash.org/viewBuildError.php?buildid=${build_id}"
  "Build Warnings (${num_warnings})"  "https://open.cdash.org/viewBuildError.php?type=1&buildid=${build_id}")

if (build_result)
  message(FATAL_ERROR
    "Failed to build")
endif ()

if ("$ENV{CTEST_NO_WARNINGS_ALLOWED}" AND num_warnings GREATER 0)
  message(FATAL_ERROR
    "Found ${num_warnings} warnings (treating as fatal).")
endif ()
file(WRITE "$ENV{CI_PROJECT_DIR}/.gitlab/num_warnings.txt" "${num_warnings}\n")

if (ctest_build_args)
  message(FATAL_ERROR
    "Failing to prevent debugging support from being committed.")
endif ()

if (NOT "$ENV{CMAKE_CI_NO_INSTALL}")
  ctest_build(APPEND
    TARGET install
    RETURN_VALUE install_result)

  if (install_result)
    message(FATAL_ERROR
      "Failed to install")
  endif ()
endif ()
