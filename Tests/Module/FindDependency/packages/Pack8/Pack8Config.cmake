include(CMakeFindDependencyMacro)

find_dependency(Pack7 REQUIRED COMPONENTS Comp1)

add_library(Pack8::Lib INTERFACE IMPORTED)
set_property(TARGET Pack8::Lib PROPERTY INTERFACE_COMPILE_DEFINITIONS HAVE_PACK8)
set_property(TARGET Pack8::Lib PROPERTY INTERFACE_LINK_LIBRARIES Pack7::Comp1)
