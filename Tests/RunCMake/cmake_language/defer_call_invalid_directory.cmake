add_subdirectory(defer_call_invalid_directory)
cmake_language(DEFER DIRECTORY defer_call_invalid_directory CALL message "Should not be allowed.")
