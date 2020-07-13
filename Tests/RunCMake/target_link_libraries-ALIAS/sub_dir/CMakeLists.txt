

add_library (lib-sub SHARED ../lib.c)
target_link_libraries (lib-sub PRIVATE alias::local)

add_executable (main-sub ../main.c)
target_link_libraries (main-sub PRIVATE alias::local)
