add_library(MathFunctions)

target_sources(MathFunctions
  PRIVATE
    MathFunctions.cxx

  PUBLIC
    FILE_SET HEADERS
    FILES
      MathFunctions.h
)

# TODO2: Add a compile feature for C++20 support to MathFunctions

# TODO3: Add a conditional which checks TUTORIAL_USE_STD_SQRT and if
#        ON, set a compile definition on MathFunctions of the same name
