set(dmg "${bin_dir}/${FOUND_FILE_1}")
execute_process(COMMAND hdiutil udifderez -xml "${dmg}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  string(REPLACE "\n" "\n  " err "  ${err}")
  message(FATAL_ERROR "Running 'hdiutil udifderez -xml' on\n  ${dmg}\nfailed with:\n${err}")
endif()
foreach(key "LPic" "STR#" "TEXT")
  if(NOT out MATCHES "<key>${key}</key>")
    string(REPLACE "\n" "\n  " out "  ${out}")
    message(FATAL_ERROR "error: running 'hdiutil udifderez -xml' on\n  ${dmg}\ndid not show '${key}' key:\n${out}")
  endif()
endforeach()
foreach(line
    # TEXT first and last base64 lines
    "\tRXhhbXBsZSBMaWNlbnNlIEZpbGUNLS0tLS0tLS0tLS0tLS0tLS0t\n"
    "\tYSBETUcuDQ0=\n"
    )
  if(NOT out MATCHES "${line}")
    string(REPLACE "\n" "\n  " out "  ${out}")
    message(FATAL_ERROR "error: running 'hdiutil udifderez -xml' on\n  ${dmg}\ndid not show '${line}':\n${out}")
  endif()
endforeach()
