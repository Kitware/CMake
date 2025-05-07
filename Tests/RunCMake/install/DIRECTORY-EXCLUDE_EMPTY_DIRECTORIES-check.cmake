file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/prefix)

execute_process(COMMAND ${CMAKE_COMMAND} -P ${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake
  OUTPUT_VARIABLE out ERROR_VARIABLE err)

set(f ${RunCMake_TEST_BINARY_DIR}/prefix/dir_to_install/empty.txt)
if(NOT EXISTS "${f}")
  string(APPEND RunCMake_TEST_FAILED
    "File was not installed:\n  ${f}\n")
endif()

set(empty_folder ${RunCMake_TEST_BINARY_DIR}/prefix/dir_to_install/empty_folder)
if(EXISTS "${empty_folder}")
  string(APPEND RunCMake_TEST_FAILED
    "empty_folder should not have be installed:\n  ${empty_folder}\n")
endif()

if(UNIX)
  set(folder_with_symlink ${RunCMake_TEST_BINARY_DIR}/prefix/dir_to_install/folder_with_symlink)
  if(NOT EXISTS "${folder_with_symlink}")
    string(APPEND RunCMake_TEST_FAILED
      "folder_with_symlink was not installed:\n  ${folder_with_symlink}\n")
  endif()

  set(symlink_to_empty_txt ${RunCMake_TEST_BINARY_DIR}/prefix/dir_to_install/folder_with_symlink/symlink_to_empty.txt)
  if(NOT EXISTS "${symlink_to_empty_txt}")
    string(APPEND RunCMake_TEST_FAILED
      "symlink_to_empty.txt was not installed:\n  ${symlink_to_empty_txt}\n")
  endif()
endif()
