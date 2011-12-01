set(file_name  ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureFile-NewLineStyle.txt)
file(WRITE ${file_name} "Data\n")
configure_file(${file_name} ${file_name}.out NEWLINE_STYLE)
