enable_language(CXX)

add_library(foo foo.cpp shader.hlsl shader2.hlsl)
set_property(TARGET foo PROPERTY VS_SOURCE_SETTINGS_FXCompile
  "$<$<CONFIG:DEBUG>:TargetProperty1=TargetProperty1ValueDebug>")
