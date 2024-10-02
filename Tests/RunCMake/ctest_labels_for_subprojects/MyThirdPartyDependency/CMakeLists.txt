cmake_minimum_required(VERSION 3.10)
project(MyThirdPartyDependency)

include(CTest)

set_directory_properties(PROPERTIES LABELS "NotASubproject;MyThirdPartyDependency")
add_subdirectory(src)
