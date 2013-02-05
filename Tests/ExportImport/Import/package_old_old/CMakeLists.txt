
cmake_minimum_required(VERSION 2.8)

find_package(testLibRequired 2.1 REQUIRED)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/main.cpp"
  "
#include \"testSharedLibRequired.h\"
int main(int argc, char **argv)
{
  TestSharedLibRequired req;
  return req.foo();
}
"
)

get_target_property(prop Req::testSharedLibRequired INTERFACE_INCLUDE_DIRECTORIES)
if (prop)
  message(SEND_ERROR "Interface of Req::testSharedLibRequired should be empty, but is ${prop}")
endif()

add_executable(old_old_test "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
target_link_libraries(old_old_test Req::testSharedLibRequired)
include_directories(${testLibRequired_INCLUDE_DIRS})
