enable_language(CXX)
add_library(foo foo.cpp)

set_property(TARGET foo PROPERTY VS_PACKAGE_REFERENCES "boost_1.7.0;SFML_2.2.0")
