set(toolchain_file "${CMAKE_BINARY_DIR}/Toolchain.cmake")
file(WRITE "${toolchain_file}" "")
foreach (var IN LISTS vars)
  file(APPEND "${toolchain_file}"
    "set(\"${var}\" [===[${${var}}]===])\n")
endforeach ()
