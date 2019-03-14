add_executable(App Main.c)
target_include_directories(App PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Lib)
target_link_libraries(App Lib)
target_compile_options(App PUBLIC "-non_shared")
