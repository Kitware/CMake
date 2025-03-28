enable_language(CXX)

add_executable(foo foo.cpp)

set_target_properties(foo PROPERTIES
    COMMON_LANGUAGE_RUNTIME "netcore"
    DOTNET_TARGET_FRAMEWORK "net8.0-windows"
    VS_FRAMEWORK_REFERENCES "Microsoft.WindowsDesktop.App.WPF")
