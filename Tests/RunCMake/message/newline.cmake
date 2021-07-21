execute_process(
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/newline-script.cmake"
  OUTPUT_FILE newline-script-stdout.txt
  ERROR_FILE newline-script-stderr.txt
  )
foreach(f out err)
  file(READ newline-script-std${f}.txt hex HEX)
  message(STATUS "${f}='${hex}'")
endforeach()
