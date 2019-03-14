enable_language(C)

add_executable(exe MACOSX_BUNDLE main.c)
add_library(lib1 SHARED obj1.c)
set_property(TARGET lib1 PROPERTY FRAMEWORK ON)

install(TARGETS exe)
install(TARGETS lib1)
