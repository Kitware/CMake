enable_language(C)

add_executable(exe_extension exe_extension.c)
add_executable(exe_no_extension exe_no_extension.c)

add_custom_target(RunScript
  ${CMAKE_COMMAND}
    -Dexe_extension=$<TARGET_FILE:exe_extension>
    -Dexe_no_extension=$<TARGET_FILE:exe_no_extension>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/WindowsNoExtensionRunScript.cmake
  DEPENDS exe_extension exe_no_extension
  )
