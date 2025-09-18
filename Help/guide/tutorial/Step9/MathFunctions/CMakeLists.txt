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
    SqrtTable

  PUBLIC
    OpAdd
    OpMul
    OpSub
)

target_compile_features(MathFunctions PRIVATE cxx_std_20)

if(TUTORIAL_USE_STD_SQRT)
  target_compile_definitions(MathFunctions PRIVATE TUTORIAL_USE_STD_SQRT)
endif()

include(CheckIncludeFiles)
check_include_files(emmintrin.h HAS_EMMINTRIN LANGUAGE CXX)

if(HAS_EMMINTRIN)
  target_compile_definitions(MathFunctions PRIVATE TUTORIAL_USE_SSE2)
endif()

include(CheckSourceCompiles)
check_source_compiles(CXX
  [=[
    typedef double v2df __attribute__((vector_size(16)));
    int main() {
      __builtin_ia32_sqrtsd(v2df{});
    }
  ]=]
  HAS_GNU_BUILTIN
)

if(HAS_GNU_BUILTIN)
  target_compile_definitions(MathFunctions PRIVATE TUTORIAL_USE_GNU_BUILTIN)
endif()

add_subdirectory(MathLogger)
add_subdirectory(MathExtensions)
add_subdirectory(MakeTable)
