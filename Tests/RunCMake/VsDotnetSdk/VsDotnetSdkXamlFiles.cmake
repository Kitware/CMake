enable_language(CSharp)

if(NOT CMAKE_CSharp_COMPILER)
    return()
endif()

include(CSharpUtilities)

add_executable(dotNetSdkWpfApp)
target_sources(dotNetSdkWpfApp
    PRIVATE
    App.xaml
    App.xaml.cs
    MainWindow.xaml
    MainWindow.xaml.cs
    Resources.Designer.cs
    Resources.resx)

csharp_set_xaml_cs_properties(
    App.xaml
    App.xaml.cs
    MainWindow.xaml
    MainWindow.xaml.cs)

csharp_set_designer_cs_properties(
    Resources.Designer.cs
    Resources.resx)

set_target_properties(dotNetSdkWpfApp
    PROPERTIES
      DOTNET_SDK "Microsoft.NET.Sdk"
      DOTNET_TARGET_FRAMEWORK "net5.0")

set_property(SOURCE App.xaml PROPERTY VS_XAML_TYPE "ApplicationDefinition")

set_property(TARGET dotNetSdkWpfApp PROPERTY VS_DOTNET_REFERENCES
    "Microsoft.CSharp"
    "PresentationCore"
    "PresentationFramework"
    "System"
    "System.Core"
    "System.Data"
    "System.Data.DataSetExtensions"
    "System.Net.Http"
    "System.Xaml"
    "System.Xml"
    "System.Xml.Linq"
    "WindowsBase")
