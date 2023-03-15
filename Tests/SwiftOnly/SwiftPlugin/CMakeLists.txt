add_executable(main main.swift)
set_target_properties(main PROPERTIES ENABLE_EXPORTS TRUE)

add_library(plugin plugin.swift)
target_link_libraries(plugin PRIVATE main)
