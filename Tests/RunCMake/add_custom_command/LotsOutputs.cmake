file(GLOB_RECURSE FILES RELATIVE "${CMAKE_ROOT}" "${CMAKE_ROOT}/Modules/*")

add_custom_command(
    OUTPUT ${FILES}
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_ROOT}/Modules" "${CMAKE_CURRENT_BINARY_DIR}/Modules"
    COMMENT "Short message"
)

add_custom_target(LotsOutputs ALL DEPENDS ${FILES})
