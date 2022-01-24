enable_language(CSharp)

add_executable(TestProgram "Program.cs")
configure_file("nuget.config.in" "nuget.config")
set_target_properties(TestProgram PROPERTIES
  VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
  VS_DOTNET_REFERENCES "System"
  VS_PACKAGE_REFERENCES "NuGetTestProject_1.0.0"
)
