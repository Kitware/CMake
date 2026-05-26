file(LOCK "${FILE_TO_LOCK}" GUARD FILE TIMEOUT 0)
message(STATUS "File locked in include")
