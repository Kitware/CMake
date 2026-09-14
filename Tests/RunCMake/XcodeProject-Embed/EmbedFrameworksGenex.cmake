# A generator expression whose result does not depend on the configuration is
# honored. Here it resolves to the name of a framework target to embed.
add_library(embedded SHARED func.m)
set_target_properties(embedded PROPERTIES FRAMEWORK TRUE)

add_executable(app MACOSX_BUNDLE main.m)
set_target_properties(app PROPERTIES
  XCODE_EMBED_FRAMEWORKS "$<1:embedded>"
)
