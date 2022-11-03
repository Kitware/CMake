add_library(SubA SubA.swift)
target_include_directories(SubA INTERFACE "$<TARGET_FILE_DIR:SubA>")
