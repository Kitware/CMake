add_library(testLibDeprecation STATIC empty.cpp)
set_property(TARGET testLibDeprecation PROPERTY DEPRECATION "Deprecated version. Please use latest version")

add_executable(testExe1 empty.cpp)
target_link_libraries(testExe1 testLibDeprecation)
