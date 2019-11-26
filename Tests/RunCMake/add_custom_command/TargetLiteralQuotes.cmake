add_library(a)
add_custom_command(TARGET a POST_BUILD COMMAND "\"b\"")
