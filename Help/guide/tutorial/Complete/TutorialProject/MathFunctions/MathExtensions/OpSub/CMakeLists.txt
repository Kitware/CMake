add_library(OpSub OBJECT)

target_sources(OpSub
  PRIVATE
    OpSub.cxx

  INTERFACE
    FILE_SET HEADERS
    FILES
      OpSub.h
)
