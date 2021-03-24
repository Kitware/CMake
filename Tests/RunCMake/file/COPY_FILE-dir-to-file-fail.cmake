set(oldname "${CMAKE_CURRENT_BINARY_DIR}/input")
set(newname "${CMAKE_CURRENT_BINARY_DIR}/output")
file(MAKE_DIRECTORY "${oldname}")
file(COPY_FILE "${oldname}" "${newname}")
