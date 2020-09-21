cmake_policy(SET CMP0053 OLD)
cmake_language(DEFER ID id1 CALL message STATUS "Deferred Message In Subdirectory: '${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE}'")
cmake_language(DEFER ID id2 CALL
  cmake_language DEFER ID id3 CALL
  message WARNING "Double-Deferred Warning In Subdirectory:\n '${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE}'")
cmake_language(DEFER ID id4 CALL include "${CMAKE_CURRENT_LIST_DIR}/include.cmake")

set(subdir_message "This should not be printed because variable evaluation is in deferred scope.")
cmake_language(DEFER DIRECTORY .. CALL message STATUS "${subdir_message}")
cmake_language(DEFER DIRECTORY .. GET_CALL_IDS ids)
message(STATUS "Immediate Message In Subdirectory: ids='${ids}'")
