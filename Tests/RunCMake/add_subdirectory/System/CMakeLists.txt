project(SystemSub NONE)

add_subdirectory(SubSub1 SYSTEM)
add_subdirectory(SubSub2)

add_library(bar STATIC bar.cpp)
if(CMAKE_Fortran_COMPILER)
  target_sources(bar PRIVATE zap.f)
endif()
# Verify CMAKE_INCLUDE_SYSTEM_FLAG_<LANG> works
target_link_libraries(bar PRIVATE subsub1bar)

add_library(foo STATIC foo.cpp)
set_target_properties(foo PROPERTIES SYSTEM OFF)

add_executable(zot zot.cpp)
