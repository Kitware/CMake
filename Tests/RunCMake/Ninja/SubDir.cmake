add_subdirectory(SubDir)
add_custom_target(TopFail ALL COMMAND does_not_exist)
