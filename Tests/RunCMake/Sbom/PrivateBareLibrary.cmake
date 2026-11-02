project(PrivateBareLibrary C)

add_library(foo STATIC ${CMAKE_CURRENT_LIST_DIR}/test.c)
target_link_libraries(foo PRIVATE m)

install(TARGETS foo EXPORT foo DESTINATION .)
