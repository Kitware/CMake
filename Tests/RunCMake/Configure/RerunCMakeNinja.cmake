set(input  ${CMAKE_CURRENT_BINARY_DIR}/input.txt)
set(stamp  ${CMAKE_CURRENT_BINARY_DIR}/stamp.txt)
file(READ ${input} content)
file(WRITE ${stamp} "${content}")
