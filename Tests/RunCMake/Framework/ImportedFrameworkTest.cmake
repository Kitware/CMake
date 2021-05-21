add_library(fw SHARED IMPORTED)
set_target_properties(fw PROPERTIES
  FRAMEWORK TRUE
  IMPORTED_LOCATION "/no/exist/fw.framework/Versions/A/fw"
  IMPORTED_SONAME "@rpath/fw.framework/Versions/A/fw"
  )

add_custom_target(print_fw ALL COMMAND
  ${CMAKE_COMMAND} -E echo "xxx$<TARGET_SONAME_FILE:fw>xxx"
  )
