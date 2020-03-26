file(GENERATE OUTPUT $<CONFIG>.txt CONTENT "$<CONFIG>\n")
file(TOUCH ${CMAKE_BINARY_DIR}/global.txt)
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "$<CONFIG>.txt;global.txt")
