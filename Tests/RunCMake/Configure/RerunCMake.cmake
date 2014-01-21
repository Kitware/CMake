set(input  ${CMAKE_CURRENT_BINARY_DIR}/CustomCMakeInput.txt)
set(stamp  ${CMAKE_CURRENT_BINARY_DIR}/CustomCMakeStamp.txt)
file(READ ${input} content)
file(WRITE ${stamp} "${content}")
