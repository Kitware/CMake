
# Import targets from the exported build tree.
include(${Import_BINARY_DIR}/../Export/ExportInterfaceBuildTree.cmake)

# Import targets from the exported install tree.
include(${CMAKE_INSTALL_PREFIX}/lib/exp/expInterface.cmake)

add_library(define_iface INTERFACE)
set_property(TARGET define_iface PROPERTY
  INTERFACE_COMPILE_DEFINITIONS DEFINE_IFACE_DEFINE)

add_executable(headeronlytest_bld headeronlytest.cpp)
target_link_libraries(headeronlytest_bld bld::headeronly)

set_property(TARGET bld::sharediface APPEND PROPERTY INTERFACE_LINK_LIBRARIES define_iface)

add_executable(interfacetest_bld interfacetest.cpp)
target_link_libraries(interfacetest_bld bld::sharediface)

include(CheckCXXSourceCompiles)

macro(do_try_compile prefix)

  set(CMAKE_REQUIRED_LIBRARIES ${prefix}headeronly)
  check_cxx_source_compiles(
    "
  #include \"headeronly.h\"

  #ifndef HEADERONLY_DEFINE
  #error Expected HEADERONLY_DEFINE
  #endif

  int main(int,char**)
  {
    HeaderOnly ho;
    return ho.foo();
  }
  " ${prefix}IFACE_TRY_COMPILE)

  if(NOT ${prefix}IFACE_TRY_COMPILE)
    message(SEND_ERROR "${prefix} try_compile with IMPORTED INTERFACE target failed!\n\n${OUTPUT}")
  endif()
endmacro()

do_try_compile(bld::)

add_executable(headeronlytest_exp headeronlytest.cpp)
target_link_libraries(headeronlytest_exp exp::headeronly)

set_property(TARGET exp::sharediface APPEND PROPERTY INTERFACE_LINK_LIBRARIES define_iface)

add_executable(interfacetest_exp interfacetest.cpp)
target_link_libraries(interfacetest_exp exp::sharediface)

do_try_compile(exp::)
