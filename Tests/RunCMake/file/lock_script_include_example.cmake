include("${CMAKE_CURRENT_LIST_DIR}/lock_include_example.cmake")

# Since the scope is 'GUARD FILE', after the 'include()' call, the file should
# be in an unlocked state
file(LOCK "${FILE_TO_LOCK}" GUARD FILE TIMEOUT 0)
message(STATUS "File locked in script with include")
