add_executable(app MACOSX_BUNDLE main.m)

set_target_properties(app PROPERTIES
  XCODE_EMBED_FRAMEWORKS "$<$<CONFIG:Debug>:foo.framework>"
)
