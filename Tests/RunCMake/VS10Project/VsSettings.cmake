enable_language(CXX)

add_library(foo foo.cpp shader.hlsl)
set_property(SOURCE shader.hlsl PROPERTY VS_SETTINGS
  "$<$<CONFIG:DEBUG>:SourceProperty1=SourceProperty1Value>")
