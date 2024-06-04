# Support for framework-specific dependencies were introduced in 3.30
# TODO: update this version req to 3.30
cmake_minimum_required(VERSION 3.29.20240531)
project(CPackNugetGenerator)

install(FILES notactuallyalib.dll DESTINATION lib/net4)
install(FILES alsonotalib.dll DESTINATION lib/net6.0)

# Create NuGet package
set(CPACK_GENERATOR NuGet)
set(CPACK_PACKAGE_NAME GeneratorTest)
set(CPACK_PACKAGE_VERSION "1.2.3")
set(CPACK_PACKAGE_VENDOR "ACME Inc")
set(CPACK_NUGET_PACKAGE_OWNERS "ACME Inc")
set(CPACK_PACKAGE_DESCRIPTION "A NuGet package for testing CMake's CPack NuGet generator")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A test NuGet package")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.example.com")
set(CPACK_NUGET_PACKAGE_REPOSITORY_URL "https://github.com/example/nugetlib.git")
set(CPACK_NUGET_PACKAGE_REPOSITORY_TYPE git)
set(CPACK_NUGET_PACKAGE_LICENSE_EXPRESSION "MIT")

# Set up dependencies
set(CPACK_NUGET_PACKAGE_TFMS "net4;net6.0")
set(CPACK_NUGET_PACKAGE_DEPENDENCIES_net4 "Foo;Bar")
# NB: If a version number is omitted, the dependency will not be created
set(CPACK_NUGET_PACKAGE_DEPENDENCIES_net4_Foo_VERSION "1.23")
set(CPACK_NUGET_PACKAGE_DEPENDENCIES_net4_Bar_VERSION "4.3.2")
# NB: General dependencies (not framework-specific) go in this variable
set(CPACK_NUGET_PACKAGE_DEPENDENCIES "Baz")
set(CPACK_NUGET_PACKAGE_DEPENDENCIES_Baz_VERSION "9.8.6")
# NB: Since "net6.0" was listed but no dependencies have been specified, an empty group
# will be added to the nuspec file for this framework. This can be used to address `NU5128`_.

include(CPack)
