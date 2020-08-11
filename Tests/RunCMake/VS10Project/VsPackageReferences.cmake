enable_language(CXX)
set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")

add_library(foo foo.cpp)

set_property(TARGET foo PROPERTY VS_PACKAGE_REFERENCES "boost_1.7.0;SFML_2.2.0")

# install and export the targets to test the correct behavior
# nuget restore will only work with an install target when the correct
# target framework version is set
set(INSTALL_CMAKE_DIR CMake)
install(TARGETS foo EXPORT foo_Export_Target)
install(EXPORT foo_Export_Target DESTINATION ${INSTALL_CMAKE_DIR} FILE fooConfig.cmake)
