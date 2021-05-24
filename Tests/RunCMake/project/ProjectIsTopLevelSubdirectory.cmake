# no project() call, includer already calls project(${RunCMake_TEST} NONE)
message(STATUS "PROJECT_IS_TOP_LEVEL=${PROJECT_IS_TOP_LEVEL}")

add_subdirectory(ProjectIsTopLevelSubdirectory)

message(STATUS "PROJECT_IS_TOP_LEVEL=${PROJECT_IS_TOP_LEVEL}")
message(STATUS "${RunCMake_TEST}_IS_TOP_LEVEL=${${RunCMake_TEST}_IS_TOP_LEVEL}")
message(STATUS "NotTopLevel_IS_TOP_LEVEL=${NotTopLevel_IS_TOP_LEVEL}")
