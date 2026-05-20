set(FILE_TO_LOCK "${CMAKE_CURRENT_BINARY_DIR}/file.lock")

file(LOCK "${FILE_TO_LOCK}" GUARD FILE TIMEOUT 0)
message(STATUS "File locked before include")
file(LOCK "${FILE_TO_LOCK}" RELEASE)

include("${CMAKE_CURRENT_LIST_DIR}/lock_include_example.cmake")

# Since the scope is 'GUARD FILE', after the 'include()' call, the file should
# be in an unlocked state
file(LOCK "${FILE_TO_LOCK}" GUARD FILE TIMEOUT 0)
message(STATUS "File locked after include")
file(LOCK "${FILE_TO_LOCK}" RELEASE)
