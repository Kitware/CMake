include(RunCMake)

# Early bailouts.
if(RunCMake_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_LESS 6.1)
  run_cmake(XcodeTooOld)
  return()
elseif(NOT CMake_TEST_Swift)
  return()
elseif(NOT RunCMake_GENERATOR MATCHES "^Ninja|^Xcode$")
  run_cmake(NotSupported)
  return()
endif()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_CONFIGURATION_TYPES=Debug\\;Release")
endif()

block()
  run_cmake(CMP0157-NEW)
  run_cmake(CMP0157-WARN)

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0157-OLD-build)

  run_cmake(CMP0157-OLD)

  if(RunCMake_GENERATOR MATCHES "Ninja.*")
    set(RunCMake_TEST_NO_CLEAN 1)
    # -n: dry-run to avoid actually compiling, -v: verbose to capture executed command
    run_cmake_command(CMP0157-OLD-build ${CMAKE_COMMAND} --build . -- -n -v)
  endif()
endblock()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SwiftSimple-build)
  run_cmake(SwiftSimple)
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG AND
      # Older Xcode versions didn't support Swift static libraries.
      NOT (RunCMake_GENERATOR STREQUAL "Xcode" AND XCODE_VERSION VERSION_LESS 9.0))
    # Check that .swiftmodule files get their own directories
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(SwiftSimple-build-Debug ${CMAKE_COMMAND} --build . --config Debug)
    run_cmake_command(SwiftSimple-build-Release ${CMAKE_COMMAND} --build . --config Release)

    # Will fail if either path doesn't exist. Passing -r because Xcode
    # generates .swiftmodule directories.
    run_cmake_command(SwiftSimple-verify ${CMAKE_COMMAND} -E
      rm -r Debug/L.swiftmodule Release/L.swiftmodule)
  endif()
endblock()

if(RunCMake_GENERATOR MATCHES "Ninja")
  block()
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
      run_cmake_with_options(Win32ExecutableDisallowed)
    else()
      run_cmake_with_options(Win32ExecutableIgnored)
      list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_SYSTEM_NAME=Darwin)
      run_cmake(SwiftMultiArch)
    endif()
  endblock()

  # Test that a second build with no changes does nothing.
  block()
    run_cmake(NoWorkToDo)
    set(RunCMake_TEST_NO_CLEAN 1)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/NoWorkToDo-build)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(NoWorkToDo-build ${CMAKE_COMMAND} --build .)
    run_cmake_command(NoWorkToDo-nowork ${CMAKE_COMMAND} --build . -- -d explain)
    file(WRITE ${RunCMake_TEST_BINARY_DIR}/hello.swift "//No-op change\n")
    run_cmake_command(NoWorkToDo-norelink ${CMAKE_COMMAND} --build . -- -d explain)
    run_cmake_command(NoWorkToDo-nowork ${CMAKE_COMMAND} --build . -- -d explain)
  endblock()

  # Test that intermediate static libraries are rebuilt when the public
  # interface of their dependency changes
  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/IncrementalSwift-build)
    # Since files are modified during test, the files are created in the cmake
    # file into the build directory
    run_cmake(IncrementalSwift)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(IncrementalSwift-first ${CMAKE_COMMAND} --build .)

    # Modify public interface of libA requiring rebuild of libB
    file(WRITE ${RunCMake_TEST_BINARY_DIR}/a.swift
      "public func callA() -> Float { return 32.0 }\n")

    # Note: We still expect this to fail, but instead of failure at link time,
    # it should fail while re-compiling libB because the function changed
    run_cmake_command(IncrementalSwift-second ${CMAKE_COMMAND} --build . -- -d explain)
  endblock()

  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CompileCommands-build)
    run_cmake(CompileCommands)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(CompileCommands-check ${CMAKE_COMMAND} --build .)
  endblock()

  block()
    # Try enabling Swift with a static-library try-compile
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/StaticLibTryCompile-build)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY)
    run_cmake(EnableSwift)
  endblock()

  block()
    # Try enabling Swift with an executable try-compile
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ExecutableTryCompile-build)
    list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_TRY_COMPILE_TARGET_TYPE=EXECUTABLE)
    run_cmake(EnableSwift)
  endblock()

  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ForceResponseFile-build)
    run_cmake(ForceResponseFile)
    set(RunCMake_TEST_NO_CLEAN 1)
    # -v: verbose to capture executed commands -n: dry-run to avoid actually compiling
    run_cmake_command(ForceResponseFile-check ${CMAKE_COMMAND} --build . -- -vn)
  endblock()

  block()
    if(CMAKE_SYSTEM_NAME MATCHES Windows)
      set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ImportLibraryFlags-build)
      run_cmake(ImportLibraryFlags)
      set(RunCMake_TEST_NO_CLEAN 1)
      run_cmake_command(ImportLibraryFlags-check ${CMAKE_COMMAND} --build . -- -n -v)
    endif()
  endblock()

  block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/SwiftLibraryModuleCommand-build)
    run_cmake(SwiftLibraryModuleCommand)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(SwiftLibraryModuleCommand-check ${CMAKE_COMMAND} --build . -- -n -v)
  endblock()
endif()
