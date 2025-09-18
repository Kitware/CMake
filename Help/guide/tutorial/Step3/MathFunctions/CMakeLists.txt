add_library(MathFunctions)

target_sources(MathFunctions
  PRIVATE
    MathFunctions.cxx

  PUBLIC
    FILE_SET HEADERS
    FILES
      MathFunctions.h
)
