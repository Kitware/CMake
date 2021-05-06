set(dst "${CMAKE_CURRENT_BINARY_DIR}/dst")
set(src "${CMAKE_CURRENT_BINARY_DIR}/src")
set(lnk "${CMAKE_CURRENT_BINARY_DIR}/lnk")

file(MAKE_DIRECTORY "${dst}")
file(MAKE_DIRECTORY "${src}")
file(CREATE_LINK "${src}" "${lnk}" SYMBOLIC)

install(FILES "${lnk}" DESTINATION "${dst}")
