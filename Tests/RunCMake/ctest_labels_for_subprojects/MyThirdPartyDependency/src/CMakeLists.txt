cmake_minimum_required(VERSION 3.10)
project(MyThirdPartyDependency)

include(CTest)

add_executable(thirdparty thirdparty.c)
add_test(NAME thirdparty COMMAND thirdparty)

set_property(TARGET thirdparty PROPERTY LABELS NotASubproject)
set_property(TEST thirdparty PROPERTY LABELS NotASubproject)
