cmake_minimum_required(VERSION 3.28)
project(rtltest CXX RC)

# A console exe that uses libc++ (std::string), so the runtime-library choice is
# observable in its imports.  It also carries a .rc resource -- which a RAD
# Studio WIN32 app almost always has -- to guard against the runtime-library
# handling mishandling a non-C/CXX language (the RC "compiler" has no runtime
# library).  RTL_VALUE is passed as Static, Dynamic, or empty (the default).
add_executable(rtltest main.cpp app.rc)

# A module library takes the same runtime-library choice, and must also be
# compiled with __DLL__ defined.
add_library(rtlmod MODULE module.cpp)

if(NOT RTL_VALUE STREQUAL "")
  set_property(TARGET rtltest rtlmod
    PROPERTY EMBT_RUNTIME_LIBRARY "${RTL_VALUE}")
endif()
