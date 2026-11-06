set(CTEST_BUILD_NAME "Step2: launchers")
site_name(CTEST_SITE)
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/../Project")
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_NOTES_FILES "${CMAKE_CURRENT_LIST_FILE}")

set(CTEST_UPDATE_COMMAND "git")
set(CTEST_UPDATE_VERSION_ONLY 1)

# Enable CTest launchers.  This allows us to capture more granular build
# warning/error information by wrapping each build command.
set(CTEST_USE_LAUNCHERS 1)

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")

ctest_start(Experimental)
ctest_update()

# Add a target that fails to compile, to show how launchers report it.
ctest_configure(OPTIONS "-DDASHBOARD_EXAMPLE_BUILD_ERROR=ON")

ctest_build()
ctest_test()
ctest_submit()
