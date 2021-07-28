enable_language(C)

add_compile_definitions(TEST_INHERITTEST)
string(APPEND CMAKE_C_FLAGS " -DTESTFLAG=\\\"TEST_INHERITTEST\\\"")

add_executable(inherit_test main.c)

target_link_libraries(inherit_test PRIVATE "TEST_INHERITTEST")
