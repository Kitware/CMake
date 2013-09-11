
cmake_minimum_required(VERSION 2.8)

project(InterfaceLibrary)

add_library(iface_nodepends INTERFACE)
target_compile_definitions(iface_nodepends INTERFACE IFACE_DEFINE)

add_executable(InterfaceLibrary definetestexe.cpp)
target_link_libraries(InterfaceLibrary iface_nodepends)

add_subdirectory(libsdir)

add_executable(sharedlibtestexe sharedlibtestexe.cpp)
target_link_libraries(sharedlibtestexe shared_iface)
