include(${CMAKE_CURRENT_LIST_DIR}/Setup.cmake)

add_library(foo STATIC ${CMAKE_CURRENT_LIST_DIR}/test.c)
add_executable(baz ${CMAKE_CURRENT_LIST_DIR}/main.c)

target_link_libraries(baz PRIVATE foo)
install(TARGETS baz EXPORT baz DESTINATION .)
