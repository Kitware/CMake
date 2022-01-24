cmake_minimum_required(VERSION 3.12)

project(NuGetTestProject VERSION 1.0.0 LANGUAGES CSharp)

add_library(NuGetPackage SHARED "Library.cs")
set_target_properties(NuGetPackage PROPERTIES
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
  VS_DOTNET_REFERENCES "System")
install(TARGETS NuGetPackage)

set(CPACK_GENERATOR "NuGet")
set(CPACK_PACKAGE_NAME "NuGetTestProject")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "Package to test automatic NuGet package restore.")
set(CPACK_PACKAGE_VENDOR "CMake.org")

include(CPack)
