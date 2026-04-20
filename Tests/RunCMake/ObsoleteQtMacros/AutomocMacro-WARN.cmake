
find_package(Qt4 REQUIRED)

cmake_diagnostic(SET CMD_DEPRECATED WARN)

add_library(foo SHARED empty.cpp)
qt4_automoc(foo_moc_srcs empty.cpp)
