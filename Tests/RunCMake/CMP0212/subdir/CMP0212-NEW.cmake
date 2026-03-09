cmake_policy(SET CMP0212 NEW)
include(CMP0212-common-custom.cmake)

# Define an executable with a different target name so that the above custom
# command finds the file-level dependency instead of the target-level
# dependency, and thus fails the build by compiling a bad C file.
add_executable(bar bad.c)
set_target_properties(bar PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_CURRENT_BINARY_DIR}>"
  RUNTIME_OUTPUT_NAME foo
)
