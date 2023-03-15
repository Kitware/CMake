set(oldname "${CMAKE_CURRENT_BINARY_DIR}/input")
set(newname "${CMAKE_CURRENT_BINARY_DIR}/output-missing/output")
file(WRITE "${oldname}" "")
file(COPY_FILE "${oldname}" "${newname}")
