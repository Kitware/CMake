add_executable(App Main.c)
target_link_libraries(App Lib)
target_compile_options(App PUBLIC "-non_shared")
