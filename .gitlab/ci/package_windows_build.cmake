cmake_minimum_required(VERSION 3.24)
include(build/ci_package_info.cmake)

set(build "${CMAKE_CURRENT_BINARY_DIR}/build")

file(GLOB paths RELATIVE "${CMAKE_CURRENT_BINARY_DIR}"
  # Allow CPack to find CMAKE_ROOT.
  "${build}/CMakeFiles/CMakeSourceDir.txt"

  # We need the main binaries.
  "${build}/bin"

  # Pass through the documentation.
  "${build}/install-doc"

  # CPack configuration.
  "${build}/CPackConfig.cmake"
  "${build}/CMakeCPackOptions.cmake"
  "${build}/Source/QtDialog/QtDialogCPack.cmake"

  # CPack/IFW packaging files.
  "${build}/CMake*.qs"

  # CPack/WIX packaging files.
  "${build}/Utilities/Release/WiX/custom_action_dll*.wxs"
  "${build}/Utilities/Release/WiX/CustomAction/CMakeWiXCustomActions.*"
  )

file(GLOB_RECURSE paths_recurse RELATIVE "${CMAKE_CURRENT_BINARY_DIR}"
  # Install rules.
  "${build}/cmake_install.cmake"
  "${build}/*/cmake_install.cmake"
  )

# Create a "package" containing the build-tree files needed to build a package.
file(MAKE_DIRECTORY build/unsigned)
file(ARCHIVE_CREATE
  OUTPUT build/unsigned/${CPACK_PACKAGE_FILE_NAME}.build.zip
  PATHS ${paths} ${paths_recurse}
  FORMAT zip
  )
