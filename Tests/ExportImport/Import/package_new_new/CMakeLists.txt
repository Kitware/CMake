
cmake_minimum_required(VERSION 2.8)

find_package(testLibRequired 2.5 REQUIRED)

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
if (NOT prop)
  message(SEND_ERROR "Interface of Req::testSharedLibRequired should not be empty")
endif()

add_executable(new_new_test "${CMAKE_CURRENT_BINARY_DIR}/main.cpp")
target_link_libraries(new_new_test Req::testSharedLibRequired)
