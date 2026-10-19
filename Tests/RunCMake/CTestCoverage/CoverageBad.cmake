# Never write to the file system root.
if(NOT DEFINED ENV{CMAKE_LISTFILE_COVERAGE_DIR})
  message(FATAL_ERROR "CMAKE_LISTFILE_COVERAGE_DIR is not set")
endif()
# A record with no end_of_record line.
file(WRITE "$ENV{CMAKE_LISTFILE_COVERAGE_DIR}/bad.info" "SF:/x\nDA:1,1\n")
