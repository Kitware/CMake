enable_language(ISPC)
enable_language(CXX)
set(CMAKE_VERBOSE_MAKEFILE TRUE)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_ISPC_FLAGS "--arch=x86")
endif()
add_executable(main main.cxx test.ispc)
