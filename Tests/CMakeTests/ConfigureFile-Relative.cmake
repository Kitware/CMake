file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureFile-Relative-In.txt "Relative test file\n")
configure_file(ConfigureFile-Relative-In.txt ConfigureFile-Relative-Out.txt)
file(READ ${CMAKE_CURRENT_BINARY_DIR}/ConfigureFile-Relative-Out.txt out)
message("${out}")
