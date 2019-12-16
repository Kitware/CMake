enable_language(C)

add_executable(exe main.c)
add_library(mylib STATIC simplelib.c)

install(TARGETS exe DESTINATION bin/$<CONFIG>)
install(TARGETS mylib DESTINATION lib/$<CONFIG>)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(exe mylib)
