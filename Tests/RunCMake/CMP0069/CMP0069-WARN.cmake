set(_CMAKE_IPO_LEGACY_BEHAVIOR NO)

add_executable(foo main.cpp)
set_target_properties(foo PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
