enable_language(C)

add_library(A STATIC BasicA.c BasicX.c)
add_library(B STATIC BasicB.c BasicX.c)
add_library(C STATIC BasicC.c BasicX.c)
target_link_libraries(B PRIVATE A)
target_link_libraries(C PRIVATE A)
target_compile_definitions(A PRIVATE BASIC_ID="A")
target_compile_definitions(B PRIVATE BASIC_ID="B")
target_compile_definitions(C PRIVATE BASIC_ID="C")

add_executable(main Basic.c)
target_link_libraries(main PRIVATE A B C)
set_property(TARGET main PROPERTY LINK_DEPENDS_DEBUG_MODE 1) # undocumented
set_property(TARGET main PROPERTY RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}>")
