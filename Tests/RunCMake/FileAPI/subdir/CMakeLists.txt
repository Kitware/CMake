target_compile_definitions(c_subdir PRIVATE SUBDIR)
target_include_directories(c_subdir PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c_subdir PRIVATE c_lib)
target_sources(c_subdir PRIVATE empty.c)
