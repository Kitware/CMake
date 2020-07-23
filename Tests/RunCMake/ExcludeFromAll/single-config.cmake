enable_language(C)
add_executable(tool main.c)
set_property(TARGET tool PROPERTY EXCLUDE_FROM_ALL "${TOOL_EXCLUDE_FROM_ALL}")

file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/target_files_$<CONFIG>.cmake" CONTENT [[
set(TARGET_FILE_tool_$<CONFIG> [==[$<TARGET_FILE:tool>]==])
]])
