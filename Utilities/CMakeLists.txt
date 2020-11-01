# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

subdirs(Doxygen)

if (CMake_DOC_ARTIFACT_PREFIX)
  # Undocumented option for CI usage to reuse already
  # built documentation.
  install(DIRECTORY ${CMake_DOC_ARTIFACT_PREFIX}/
          DESTINATION . USE_SOURCE_PERMISSIONS)
else()
  # Normal documentation build.
  add_subdirectory(Sphinx)
endif()

if(WIX_CUSTOM_ACTION_ENABLED)
  add_subdirectory(Release/WiX)
endif()

# Make sure generated files use the same clang-tidy checks (none).
configure_file(.clang-tidy .clang-tidy COPYONLY)
