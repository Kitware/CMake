enable_language(C)

add_executable(per_config_c per_config_c.c
  "$<$<CONFIG:Debug>:per_config_c_debug.c>"
  "$<$<NOT:$<CONFIG:Debug>>:per_config_c_other.c>"
  )

set_target_properties(per_config_c PROPERTIES UNITY_BUILD ON)
target_compile_definitions(per_config_c PRIVATE
  "$<$<CONFIG:Debug>:CFG_DEBUG>"
  "$<$<NOT:$<CONFIG:Debug>>:CFG_OTHER>"
  )
