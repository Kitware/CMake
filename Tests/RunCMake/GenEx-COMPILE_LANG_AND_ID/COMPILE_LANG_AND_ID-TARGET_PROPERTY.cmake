enable_language(C)

add_library (lib SHARED empty.c)
set_target_properties(lib PROPERTIES
  INCLUDE_DIRECTORIES "$<$<COMPILE_LANG_AND_ID:C,GNU>:/usr/include>"
  COMPILE_DEFINITIONS "$<$<COMPILE_LANG_AND_ID:C,GNU>:DEF>"
  COMPILE_OPTIONS "$<$<COMPILE_LANG_AND_ID:C,GNU>:-O>")

add_custom_target(drive
  COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:lib,INCLUDE_DIRECTORIES>
                                   $<TARGET_PROPERTY:lib,COMPILE_DEFINITIONS>
                                   $<TARGET_PROPERTY:lib,COMPILE_OPTIONS>)

add_custom_command(TARGET drive PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:lib,INCLUDE_DIRECTORIES>
                                   $<TARGET_PROPERTY:lib,COMPILE_DEFINITIONS>
                                   $<TARGET_PROPERTY:lib,COMPILE_OPTIONS>)
