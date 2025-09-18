add_library(OpMul OBJECT)

target_sources(OpMul
  PRIVATE
    OpMul.cxx

  INTERFACE
    FILE_SET HEADERS
    FILES
      OpMul.h
)
