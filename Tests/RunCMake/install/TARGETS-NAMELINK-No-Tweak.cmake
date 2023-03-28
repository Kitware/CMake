enable_language(C)

add_library(foo SHARED obj1.c)
set_target_properties(foo PROPERTIES
  VERSION 1.0
  SOVERSION 1
  INSTALL_RPATH "$ORIGIN"
  )
install(TARGETS foo DESTINATION lib)

# Replace the .so "namelink" symlink with a linker script.
# It is no longer a symlink, so any install tweaks would break.
# This verifies that no install tweaks are added for the namelink.
set(linker_script "INPUT($<TARGET_SONAME_FILE_NAME:foo>)")
add_custom_command(TARGET foo POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E remove "$<TARGET_LINKER_FILE:foo>"
  COMMAND "${CMAKE_COMMAND}" -E echo "${linker_script}" > "$<TARGET_LINKER_FILE:foo>"
  COMMENT "Generating linker script: '${linker_script}' as file $<TARGET_LINKER_FILE:foo>"
  VERBATIM
  )
