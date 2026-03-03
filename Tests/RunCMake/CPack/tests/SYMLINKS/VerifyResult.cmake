set(whitespaces "[\\t\\n\\r ]*")

#######################
# verify generated symbolic links
#######################
file(GLOB_RECURSE symlink_files RELATIVE "${bin_dir}" "${bin_dir}/*/symlink_*")

foreach(check_symlink IN LISTS symlink_files)
  cmake_path(GET check_symlink FILENAME symlink_name)
  file(READ_SYMLINK "${check_symlink}" symlink_dst)

  if("${symlink_name}" STREQUAL "symlink_to_empty_dir")
    string(REGEX MATCH "^empty_dir$" check_symlink "${symlink_dst}")
  elseif("${symlink_name}" STREQUAL "symlink_to_non_empty_dir")
    string(REGEX MATCH "^non_empty_dir$" check_symlink "${symlink_dst}")
  else()
    message(FATAL_ERROR "error: unexpected rpm symbolic link '${check_symlink}'")
  endif()

  if(NOT check_symlink)
    message(FATAL_ERROR "symlink points to unexpected location '${symlink_dst}'")
  endif()
endforeach()
