
cmake_minimum_required(VERSION 2.8)

find_package(testLibRequired 2.5 REQUIRED)

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_LIBRARIES Req::testSharedLibRequired)
check_cxx_source_compiles(
  "
#include \"testSharedLibRequired.h\"
int main(int argc, char **argv)
{
  TestSharedLibRequired req;
  return req.foo();
}
" SHARED_LIB_REQUIRED)

if(NOT SHARED_LIB_REQUIRED)
  message(SEND_ERROR "try_compile with IMPORTED targets failed!\n\n${OUTPUT}")
endif()
