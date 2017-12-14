
enable_language(CXX)

add_executable(main main.cpp)
set_property(SOURCE main.cpp PROPERTY COMPILE_DEFINITIONS $<$<COMPILE_LANGUAGE:CXX>:ANYTHING>)
