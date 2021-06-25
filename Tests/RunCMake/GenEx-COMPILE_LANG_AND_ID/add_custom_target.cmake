
add_custom_target(drive
  COMMAND ${CMAKE_COMMAND} -E echo $<COMPILE_LANG_AND_ID:LANG,ID>
)
