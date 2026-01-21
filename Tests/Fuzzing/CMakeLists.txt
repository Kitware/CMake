# Fuzzing targets for CMake
# See README.rst for documentation.

# Determine fuzzing engine
# OSS-Fuzz sets LIB_FUZZING_ENGINE, otherwise use libFuzzer
if(DEFINED ENV{LIB_FUZZING_ENGINE})
  set(FUZZING_ENGINE $ENV{LIB_FUZZING_ENGINE})
  set(FUZZING_ENGINE_FOUND TRUE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Check if libFuzzer is available (needs both compile and link flags)
  include(CheckCXXSourceCompiles)
  set(CMAKE_REQUIRED_FLAGS "-fsanitize=fuzzer")
  set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=fuzzer")
  check_cxx_source_compiles("extern \"C\" int LLVMFuzzerTestOneInput(const char *data, long size) { return 0; }" HAVE_LIBFUZZER)
  unset(CMAKE_REQUIRED_FLAGS)
  unset(CMAKE_REQUIRED_LINK_OPTIONS)
  if(HAVE_LIBFUZZER)
    set(FUZZING_ENGINE "-fsanitize=fuzzer")
    set(FUZZING_ENGINE_FOUND TRUE)
  endif()
endif()

if(NOT FUZZING_ENGINE_FOUND)
  message(FATAL_ERROR "No fuzzing engine found. CMake_BUILD_FUZZING requires libFuzzer or LIB_FUZZING_ENGINE.")
endif()

# Common link libraries
set(FUZZER_LINK_LIBS
  CMakeLib
)

# Macro to add a fuzzer target
macro(add_fuzzer name source)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE ${FUZZER_LINK_LIBS})

  # If using libFuzzer directly, add the flag
  if(FUZZING_ENGINE STREQUAL "-fsanitize=fuzzer")
    target_compile_options(${name} PRIVATE -fsanitize=fuzzer)
    target_link_options(${name} PRIVATE -fsanitize=fuzzer)
  else()
    # OSS-Fuzz provides engine as a library
    target_link_libraries(${name} PRIVATE ${FUZZING_ENGINE})
  endif()

  # Ensure we don't apply clang-tidy to fuzzers
  set_property(TARGET ${name} PROPERTY C_CLANG_TIDY "")
  set_property(TARGET ${name} PROPERTY CXX_CLANG_TIDY "")
endmacro()

# Existing fuzzer from OSS-Fuzz integration
add_fuzzer(xml_parser_fuzzer xml_parser_fuzzer.cc)

message(STATUS "Fuzzing targets enabled with engine: ${FUZZING_ENGINE}")

# CMakeLists.txt lexer fuzzer
add_fuzzer(cmListFileLexerFuzzer cmListFileLexerFuzzer.cxx)

# CMakeLists.txt parser fuzzer
add_fuzzer(cmListFileParserFuzzer cmListFileParserFuzzer.cxx)

# Generator expression fuzzer
add_fuzzer(cmGeneratorExpressionFuzzer cmGeneratorExpressionFuzzer.cxx)

# Math expression parser fuzzer
add_fuzzer(cmExprParserFuzzer cmExprParserFuzzer.cxx)

# pkg-config parser fuzzer
add_fuzzer(cmPkgConfigParserFuzzer cmPkgConfigParserFuzzer.cxx)

# JSON parser fuzzer
add_fuzzer(cmJSONParserFuzzer cmJSONParserFuzzer.cxx)

# GCC dependency file fuzzer
add_fuzzer(cmGccDepfileFuzzer cmGccDepfileFuzzer.cxx)

# String algorithms fuzzer
add_fuzzer(cmStringAlgorithmsFuzzer cmStringAlgorithmsFuzzer.cxx)

# Version parser fuzzer
add_fuzzer(cmVersionFuzzer cmVersionFuzzer.cxx)

# CMake path fuzzer
add_fuzzer(cmCMakePathFuzzer cmCMakePathFuzzer.cxx)

# File glob fuzzer
add_fuzzer(cmGlobFuzzer cmGlobFuzzer.cxx)

# ELF binary parser fuzzer
add_fuzzer(cmELFFuzzer cmELFFuzzer.cxx)

# Archive extraction fuzzer
add_fuzzer(cmArchiveExtractFuzzer cmArchiveExtractFuzzer.cxx)

# File lock fuzzer
add_fuzzer(cmFileLockFuzzer cmFileLockFuzzer.cxx)

# CMake script fuzzer
add_fuzzer(cmScriptFuzzer cmScriptFuzzer.cxx)
