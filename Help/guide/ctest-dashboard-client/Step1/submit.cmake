# Set the name of this build to distinguish it from other builds on CDash.
set(CTEST_BUILD_NAME "Step1: introduction")

# Look up the name of this host and use it as the site name.
# Alternatively, set CTEST_SITE directly.
site_name(CTEST_SITE)

# Tell CTest where the top-level CMakeLists.txt file is, and what directory
# to use to build the project.
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/../Project")
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")

# Specify the native build system to generate.
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")

# Attach this script as a note to the build on CDash.
set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

# Perform a "version-only update" with Git.  This reports the current
# commit and any locally modified files to CDash without changing the
# source tree.
set(CTEST_UPDATE_COMMAND "git")
set(CTEST_UPDATE_VERSION_ONLY 1)

# Start from an empty build tree rather than building incrementally.
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

# Run the dashboard steps to configure, build, and test the project,
# and submit the results to CDash.
ctest_start(Experimental)
ctest_update()
ctest_configure()
ctest_build()
ctest_test()
ctest_submit()
