cmake_minimum_required(VERSION 3.10)
project(MyThirdPartyDependency)

include(CTest)

add_executable(third_party third_party.c)
add_test(NAME third_party COMMAND third_party)

set_property(TARGET third_party PROPERTY LABELS NotASubproject)
set_property(TEST third_party PROPERTY LABELS NotASubproject)
