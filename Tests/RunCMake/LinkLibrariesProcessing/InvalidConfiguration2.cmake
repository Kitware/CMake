
enable_language(C)

set(CMAKE_C_LINK_LIBRARIES_PROCESSING UNICITY=ALL ORDER)

add_library(lib STATIC lib.c)

add_executable(main main.c)
target_link_libraries(main PRIVATE lib)
