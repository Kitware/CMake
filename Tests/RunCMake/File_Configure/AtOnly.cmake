set(file_name  ${CMAKE_CURRENT_BINARY_DIR}/atonly.txt)
set(var_a "a")
set(var_b "b")
file(CONFIGURE
    OUTPUT ${file_name}
    CONTENT "-->@var_a@<-- -->${var_b}<--"
    @ONLY
)
file(READ ${file_name} file_content)
if(NOT file_content STREQUAL "-->a<-- -->${var_b}<--")
    message(FATAL_ERROR "@ONLY doesn't work")
endif()
