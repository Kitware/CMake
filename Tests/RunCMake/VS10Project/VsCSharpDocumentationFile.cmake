set(CMAKE_CONFIGURATION_TYPES Debug)
enable_language(CSharp)

add_library(foo SHARED
  foo.cs)

set_target_properties(foo PROPERTIES
  VS_DOTNET_DOCUMENTATION_FILE foo.xml)
