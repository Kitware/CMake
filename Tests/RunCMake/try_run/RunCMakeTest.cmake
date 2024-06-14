include(RunCMake)

# Detect information from the toolchain:
# - CMAKE_SYSTEM_NAME
# - CMAKE_C_COMPILER_ID
# - CMAKE_C_COMPILER_VERSION
# - CMAKE_CXX_COMPILER_ID
# - CMAKE_CXX_COMPILER_VERSION
run_cmake_with_options(Inspect
  -DCMake_TEST_Fortran=${CMake_TEST_Fortran}
  )
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_cmake(BinDirEmpty)
run_cmake(BinDirRelative)
run_cmake(NoOutputVariable)
run_cmake(ConfigureLog)

set(RunCMake_TEST_OPTIONS -Dtry_compile_DEFS=old_signature.cmake)
include(${RunCMake_SOURCE_DIR}/old_and_new_signature_tests.cmake)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS -Dtry_compile_DEFS=new_signature.cmake)
include(${RunCMake_SOURCE_DIR}/old_and_new_signature_tests.cmake)
unset(RunCMake_TEST_OPTIONS)

if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows)$" AND
    CMAKE_C_COMPILER_ID MATCHES "^(MSVC|GNU|LCC|Clang|AppleClang)$")
  set (RunCMake_TEST_OPTIONS -DRunCMake_C_COMPILER_ID=${CMAKE_C_COMPILER_ID})
  run_cmake(LinkOptions)
  unset (RunCMake_TEST_OPTIONS)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows)$" AND
    CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$" AND
    (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU" AND CMAKE_Fortran_COMPILER_VERSION VERSION_GREATER_EQUAL 4.3))
  run_cmake(LinkerLanguage)
endif()
