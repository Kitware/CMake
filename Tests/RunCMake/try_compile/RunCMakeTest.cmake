include(RunCMake)

# Detect information from the toolchain:
# - CMAKE_C_COMPILER_ID
# - CMAKE_C_COMPILER_VERSION
# - CMAKE_C_STANDARD_DEFAULT
# - CMAKE_CXX_COMPILER_ID
# - CMAKE_CXX_COMPILER_VERSION
# - CMAKE_CXX_STANDARD_DEFAULT
# - CMAKE_CXX_EXTENSIONS_DEFAULT
# - CMAKE_OBJC_STANDARD_DEFAULT
# - CMAKE_OBJCXX_STANDARD_DEFAULT
run_cmake_with_options(Inspect
  -DCMake_TEST_OBJC=${CMake_TEST_OBJC}
  )
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_cmake(ConfigureLog)
run_cmake(NoArgs)
run_cmake(OneArg)
run_cmake(TwoArgs)
run_cmake(NoSources)
run_cmake(BinDirEmpty)
run_cmake(BinDirRelative)
run_cmake(ProjectSrcDirMissing)
run_cmake(ProjectSrcDirEmpty)
run_cmake(ProjectBinDirEmpty)
run_cmake(OldProjectSrcDirEmpty)
run_cmake(OldProjectBinDirEmpty)

set(RunCMake_TEST_OPTIONS -Dtry_compile_DEFS=old_signature.cmake)
include(${RunCMake_SOURCE_DIR}/old_and_new_signature_tests.cmake)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS -Dtry_compile_DEFS=new_signature.cmake)
include(${RunCMake_SOURCE_DIR}/old_and_new_signature_tests.cmake)
unset(RunCMake_TEST_OPTIONS)

run_cmake(SourceFromOneArg)
run_cmake(SourceFromThreeArgs)
run_cmake(SourceFromBadName)
run_cmake(SourceFromBadFile)

run_cmake(ProjectCopyFile)
run_cmake(NonSourceCopyFile)
run_cmake(NonSourceCompileDefinitions)

run_cmake(Verbose)

set(RunCMake_TEST_OPTIONS --debug-trycompile)
run_cmake(PlatformVariables)
run_cmake(WarnDeprecated)
unset(RunCMake_TEST_OPTIONS)

if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows)$" AND
    CMAKE_C_COMPILER_ID MATCHES "^(MSVC|GNU|LCC|Clang|AppleClang)$")
  set (RunCMake_TEST_OPTIONS -DRunCMake_C_COMPILER_ID=${CMAKE_C_COMPILER_ID})
  run_cmake(LinkOptions)
  unset (RunCMake_TEST_OPTIONS)
endif()

run_cmake(CMP0056)
run_cmake(CMP0066)
run_cmake(CMP0067)
run_cmake(CMP0137-WARN)
run_cmake(CMP0137-NEW)

if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RerunCMake-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  set(in_tc  "${RunCMake_TEST_BINARY_DIR}/TryCompileInput.c")
  file(WRITE "${in_tc}" "int main(void) { return 0; }\n")

  # Older Ninja keeps all rerun output on stdout
  set(ninja "")
  if(RunCMake_GENERATOR STREQUAL "Ninja")
    execute_process(COMMAND ${RunCMake_MAKE_PROGRAM} --version
      OUTPUT_VARIABLE ninja_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(ninja_version VERSION_LESS 1.5)
      set(ninja -ninja-no-console)
    endif()
  endif()

  message(STATUS "RerunCMake: first configuration...")
  run_cmake(RerunCMake)
  if(NOT CMake_TEST_FILESYSTEM_1S)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(RerunCMake-nowork${ninja} ${CMAKE_COMMAND} --build .)
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1) # handle 1s resolution
  message(STATUS "RerunCMake: modify try_compile input...")
  file(WRITE "${in_tc}" "does-not-compile\n")
  run_cmake_command(RerunCMake-rerun${ninja} ${CMAKE_COMMAND} --build .)
  if(NOT CMake_TEST_FILESYSTEM_1S)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(RerunCMake-nowork${ninja} ${CMAKE_COMMAND} --build .)
    unset(RunCMake_TEST_OUTPUT_MERGE)
  endif()

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endif()

# FIXME: Support more compilers and default standard levels.
if (DEFINED CMAKE_CXX_STANDARD_DEFAULT AND
    DEFINED CMAKE_CXX_EXTENSIONS_DEFAULT AND (
    (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.7) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    ))
  run_cmake(CMP0128-WARN)
  if(NOT CMAKE_CXX_STANDARD_DEFAULT EQUAL 11)
    run_cmake(CMP0128-NEW)
  endif()
endif()

if(UNIX)
  run_cmake(CleanupNoFollowSymlink)
endif()
