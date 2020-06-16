
add_custom_target(drive
  COMMAND ${CMAKE_COMMAND} -E echo $<LINK_LANG_AND_ID:LANG,ID>
)
