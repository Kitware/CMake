# Test that file(LOCK) does not truncate existing files when the lock path
# is a symlink pointing to another file. This is a regression test for a
# data destruction vulnerability (CWE-59).

set(target_file "${CMAKE_CURRENT_BINARY_DIR}/target_file.txt")
set(lock_symlink "${CMAKE_CURRENT_BINARY_DIR}/lock_symlink")

# Create a target file with known content
file(WRITE "${target_file}" "IMPORTANT DATA THAT MUST NOT BE DESTROYED")

# Read original content for comparison
file(READ "${target_file}" original_content)
message(STATUS "Original content: ${original_content}")

# Create a symlink pointing to the target file
file(CREATE_LINK "${target_file}" "${lock_symlink}" SYMBOLIC)

# Attempt to lock the symlink - this should NOT truncate the target
file(LOCK "${lock_symlink}" RESULT_VARIABLE lock_result)
message(STATUS "Lock result: ${lock_result}")

# Release the lock
file(LOCK "${lock_symlink}" RELEASE)

# Verify the target file still has its content
file(READ "${target_file}" final_content)
message(STATUS "Final content: ${final_content}")

if(NOT final_content STREQUAL original_content)
  message(FATAL_ERROR
    "VULNERABILITY: file(LOCK) truncated the symlink target!\n"
    "Original: '${original_content}'\n"
    "Final: '${final_content}'"
  )
endif()

message(STATUS "PASS: Symlink target was not truncated")
