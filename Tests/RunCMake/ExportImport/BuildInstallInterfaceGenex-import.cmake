enable_language(C)

find_package(build REQUIRED)
find_package(install REQUIRED)

add_library(buildlib STATIC buildlib.c)
target_link_libraries(buildlib PRIVATE build::mainlib)
add_library(installlib STATIC installlib.c)
target_link_libraries(installlib PRIVATE install::mainlib)
