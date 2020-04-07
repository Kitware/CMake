include("${RunCMake_TEST_BINARY_DIR}/FrameworkMultiConfigPostfixInfo.cmake")

get_filename_component(framework_location "${framework_dir}" DIRECTORY)
set(non_existent_debug_framework_dir "${framework_location}/${target_file_name}_debug.framework")
set(framework_resources "${framework_dir}/Resources")
set(plist_file "${framework_resources}/Info.plist")

set(symlink_release_path "${framework_dir}/${target_file_name}")
set(framework_release_path "${framework_dir}/Versions/A/${target_file_name}")

# When using a multi config generator (like Ninja Multi-Config and Xcode),
# the postfix will be applied to the debug framework library name and the symlink name.
# For single config generators, the name stays the same as the the release framework.
if(is_multi_config)
    set(symlink_debug_path "${framework_dir}/${target_file_name}_debug")
    set(framework_debug_path "${framework_dir}/Versions/A/${target_file_name}_debug")
else()
    set(symlink_debug_path "${framework_dir}/${target_file_name}")
    set(framework_debug_path "${framework_dir}/Versions/A/${target_file_name}")
endif()

if(NOT IS_DIRECTORY ${framework_dir})
  message(SEND_ERROR "Framework dir not found at ${framework_dir}")
endif()

if(IS_DIRECTORY ${non_existent_debug_framework_dir})
  message(SEND_ERROR
      "A framework dir with a debug suffix should not exist at ${non_existent_debug_framework_dir}")
endif()

if(NOT IS_SYMLINK "${symlink_release_path}")
  message(SEND_ERROR "Release framework symlink not found at ${symlink_release_path}")
endif()

if(NOT IS_SYMLINK "${symlink_debug_path}")
  message(SEND_ERROR "Debug framework symlink not found at ${symlink_debug_path}")
endif()

if(NOT EXISTS "${framework_release_path}")
  message(SEND_ERROR "Release framework not found at ${framework_release_path}")
endif()

if(NOT EXISTS "${framework_debug_path}")
  message(SEND_ERROR "Debug framework not found at ${framework_debug_path}")
endif()
