project(CxxDialect)

add_executable(use_typeof use_typeof.cxx)
set_property(TARGET use_typeof PROPERTY CXX_STANDARD 98)
set_property(TARGET use_typeof PROPERTY CXX_EXTENSIONS TRUE)

add_executable(use_constexpr use_constexpr.cxx)
set_property(TARGET use_constexpr PROPERTY CXX_STANDARD 11)

add_executable(CxxDialect use_constexpr_and_typeof.cxx)
set_property(TARGET CxxDialect PROPERTY CXX_STANDARD 11)
set_property(TARGET CxxDialect PROPERTY CXX_EXTENSIONS TRUE)
