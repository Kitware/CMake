add_library(OpAdd OBJECT)

target_sources(OpAdd
  PRIVATE
    OpAdd.cxx

  INTERFACE
    FILE_SET HEADERS
    FILES
      OpAdd.h
)
