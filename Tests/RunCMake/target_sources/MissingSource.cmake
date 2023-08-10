cmake_policy(SET CMP0115 NEW)
add_custom_target(foo)
target_sources(foo PRIVATE missing.txt)
