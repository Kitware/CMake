add_library(MathFunctions)

target_sources(MathFunctions
  PRIVATE
    MathFunctions.cxx

  PUBLIC
    FILE_SET HEADERS
    FILES
      MathFunctions.h
)

# TODO3: Add a link to MathLogger for the MathFunctions library. Note that
#        MathLogger will only be used in the MathFunctions implementation,
#        not the headers

# TODO8: Add links to OpAdd, OpMul, and OpSub libraries for the
#        MathFunctions library.  Note that their headers will be
#        exposed in the MathFunctions.h header, and must be available
#        to consumers.

target_compile_features(MathFunctions PRIVATE cxx_std_20)

if(TUTORIAL_USE_STD_SQRT)
  target_compile_definitions(MathFunctions PRIVATE TUTORIAL_USE_STD_SQRT)
endif()

# TODO4: Add the MathLogger subdirectory

# TODO9: Add the MathExtensions subdirectory
