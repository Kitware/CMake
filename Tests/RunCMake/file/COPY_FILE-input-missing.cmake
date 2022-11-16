set(oldname "${CMAKE_CURRENT_BINARY_DIR}/input-missing")
set(newname "${CMAKE_CURRENT_BINARY_DIR}/output")
file(COPY_FILE "${oldname}" "${newname}")
