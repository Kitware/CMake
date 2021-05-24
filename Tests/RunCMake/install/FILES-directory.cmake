set(dst "${CMAKE_CURRENT_BINARY_DIR}/dst")
set(src "${CMAKE_CURRENT_BINARY_DIR}/src")

file(MAKE_DIRECTORY "${dst}")
file(MAKE_DIRECTORY "${src}")

install(FILES "${src}" DESTINATION "${dst}")
