enable_language(CSharp)

add_library(foo SHARED foo.cs )

set_target_properties(foo
 PROPERTIES
   DOTNET_TARGET_FRAMEWORK_VERSION "v3.9"
)
