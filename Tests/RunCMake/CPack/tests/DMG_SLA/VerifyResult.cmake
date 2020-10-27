set(dmg "${bin_dir}/${FOUND_FILE_1}")
execute_process(COMMAND hdiutil udifderez -xml "${dmg}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  string(REPLACE "\n" "\n  " err "  ${err}")
  message(FATAL_ERROR "Running 'hdiutil udifderez -xml' on\n  ${dmg}\nfailed with:\n${err}")
endif()
foreach(key "LPic" "STR#" "TEXT" "RTF ")
  if(NOT out MATCHES "<key>${key}</key>")
    string(REPLACE "\n" "\n  " out "  ${out}")
    message(FATAL_ERROR "error: running 'hdiutil udifderez -xml' on\n  ${dmg}\ndid not show '${key}' key:\n${out}")
  endif()
endforeach()
foreach(line
    # LPic
    "\tAAAAAgAAAAAAAAADAAEAAA==\n"
    # STR# English first and last base64 lines
    "\tAAkHRW5nbGlzaAVBZ3JlZQhEaXNhZ3JlZQVQcmludAdTYXZlLi4u\n"
    "\tZCBhIHByaW50ZXIu\n"
    # STR# German first and last base64 lines
    "\tAAkGR2VybWFuC0FremVwdGllcmVuCEFibGVobmVuB0RydWNrZW4M\n"
    "\tYXVzZ2V3wopobHQgaXN0Lg==\n"
    # RTF English first and last base64 lines
    "\te1xydGYxXGFuc2lcYW5zaWNwZzEyNTJcZGVmZjBcbm91aWNvbXBh\n"
    "\tdmlkZWQuXHBhcg1ccGFyDX0NDQ==\n"
    # TEXT German first and last base64 lines
    "\tTElaRU5aDS0tLS0tLQ1EaWVzIGlzdCBlaW4gSW5zdGFsbGF0aW9u\n"
    "\tZ2ViZW4uDQ0=\n"
    )
  if(NOT out MATCHES "${line}")
    string(REPLACE "\n" "\n  " out "  ${out}")
    message(FATAL_ERROR "error: running 'hdiutil udifderez -xml' on\n  ${dmg}\ndid not show '${line}':\n${out}")
  endif()
endforeach()
