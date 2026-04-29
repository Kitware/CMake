include(WriteBasicConfigVersionFile)

# Verify that ExactVersion remains accepted as a deprecated alias and
# generates the same config version file as SamePatchVersion.
set(_installed_version 1.2.3.4)
set(_exact_file "${CMAKE_CURRENT_BINARY_DIR}/ExactVersionConfigVersion.cmake")
set(_same_patch_file "${CMAKE_CURRENT_BINARY_DIR}/SamePatchVersionConfigVersion.cmake")

write_basic_config_version_file(
  "${_exact_file}"
  VERSION "${_installed_version}"
  COMPATIBILITY ExactVersion)

write_basic_config_version_file(
  "${_same_patch_file}"
  VERSION "${_installed_version}"
  COMPATIBILITY SamePatchVersion)

file(READ "${_exact_file}" _exact_content)
file(READ "${_same_patch_file}" _same_patch_content)

if(NOT _exact_content STREQUAL _same_patch_content)
  message(SEND_ERROR
    "ExactVersion should generate the same config version file as SamePatchVersion.")
endif()
