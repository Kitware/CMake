add_library(MathFunctions)

target_sources(MathFunctions
  PRIVATE
    MathFunctions.cxx

  PUBLIC
    FILE_SET HEADERS
    FILES
      MathFunctions.h
)

target_link_libraries(MathFunctions
  PRIVATE
    MathLogger

  PUBLIC
    OpAdd
    OpMul
    OpSub
)

target_compile_features(MathFunctions PRIVATE cxx_std_20)

if(TUTORIAL_USE_STD_SQRT)
  target_compile_definitions(MathFunctions PRIVATE TUTORIAL_USE_STD_SQRT)
endif()

# TODO1: Include the CheckIncludeFiles module and use it to check for
#        the emmintrin.h header.

# TODO2: If emmintrin.h is available, add a compile definition to MathFunctions
#        named TUTORIAL_USE_SSE2. This will only be needed by the MathFunctions
#        implementation file.

# TODO4: Include the CheckSourceCompiles module and use it to check if the
#        following program compiles:
#
#           typedef double v2df __attribute__((vector_size(16)));
#           int main() {
#             __builtin_ia32_sqrtsd(v2df{});
#           }

# TODO5: If the GNU builtins are available, add a compile definition to
#        MathFunctions named TUTORIAL_USE_GNU_BUILTIN. This will only be needed
#        by the MathFunctions implementation file.

add_subdirectory(MathLogger)
add_subdirectory(MathExtensions)
