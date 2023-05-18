project(Framework)
enable_language(CXX)

add_library(static_framework STATIC ../empty.cxx)
set_property(TARGET static_framework PROPERTY FRAMEWORK ON)

add_library(shared_framework SHARED ../empty.cxx)
set_property(TARGET shared_framework PROPERTY FRAMEWORK ON)
set_property(TARGET shared_framework PROPERTY SYSTEM ON)

add_library(import_framework SHARED IMPORTED)
set_property(TARGET import_framework PROPERTY FRAMEWORK ON)
set_property(TARGET import_framework PROPERTY IMPORTED_LOCATION /usr/Frameworks/Foo.framework/Foo)
set_property(TARGET import_framework PROPERTY IMPORTED_IMPLIB /usr/Frameworks/Foo.framework/Foo.lib)

add_executable(exe_framework ../empty.cxx)
target_link_libraries(exe_framework PRIVATE static_framework shared_framework import_framework)
