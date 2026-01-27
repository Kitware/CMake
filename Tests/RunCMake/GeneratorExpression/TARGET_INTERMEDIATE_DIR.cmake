enable_language(C)
add_executable(foo simple.c)
file(GENERATE OUTPUT TARGET_INTERMEDIATE_DIR-generated.txt
     CONTENT "$<TARGET_INTERMEDIATE_DIR:foo>")
