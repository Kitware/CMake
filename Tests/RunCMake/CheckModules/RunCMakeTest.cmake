include(RunCMake)

run_cmake(CMP0075)

run_cmake(CheckStructHasMemberOk)
run_cmake(CheckStructHasMemberUnknownLanguage)
run_cmake(CheckStructHasMemberMissingLanguage)
run_cmake(CheckStructHasMemberMissingKey)
run_cmake(CheckStructHasMemberTooManyArguments)
run_cmake(CheckStructHasMemberWrongKey)

run_cmake(CheckTypeSizeOk)
run_cmake(CheckTypeSizeUnknownLanguage)
run_cmake(CheckTypeSizeMissingLanguage)
run_cmake(CheckTypeSizeMissingResultVariable)
run_cmake(CheckTypeSizeUnknownArgument)
run_cmake(CheckTypeSizeMixedArgs)

run_cmake(CheckTypeSizeOkNoC)

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CheckTypeSize-build)
  run_cmake(CheckTypeSize)

  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_OUTPUT_MERGE 1)

  run_cmake_command(CheckTypeSize-build ${CMAKE_COMMAND} --build .)
  run_cmake_command(CheckTypeSize-build ${CMAKE_CTEST_COMMAND} -C Debug)
endblock()

run_cmake(CheckIncludeFilesOk)
run_cmake(CheckIncludeFilesOkNoC)
run_cmake(CheckIncludeFilesMissingLanguage)
run_cmake(CheckIncludeFilesUnknownArgument)
run_cmake(CheckIncludeFilesUnknownLanguage)

block()
    # Set common variables
    set(libDir ${RunCMake_BINARY_DIR}/CheckLinkDirectoriesTestLib-build/TestLib/lib)
    set(libName mySharedLibrary)

    # Build common part
    run_cmake(CheckLinkDirectoriesTestLib)
    set(RunCMake_TEST_NO_CLEAN 1)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    run_cmake_command(CheckLinkDirectoriesTestLib ${CMAKE_COMMAND} --build .)

    # Run tests cleanly
    unset(RunCMake_TEST_NO_CLEAN)
    unset(RunCMake_TEST_OUTPUT_MERGE)

    set(RunCMake_TEST_VARIANT_DESCRIPTION "WithDirectories")
    run_cmake_with_options("CheckLinkDirectories"
        "-DCMAKE_REQUIRED_LIBRARIES=${libName}"
        "-DCMAKE_REQUIRED_LINK_DIRECTORIES=${libDir}"
        "-DIS_NEED_SUCCESS:BOOL=ON"
    )
    set(RunCMake_TEST_VARIANT_DESCRIPTION "WithoutDirectories")
    run_cmake_with_options("CheckLinkDirectories"
        "-DCMAKE_REQUIRED_LIBRARIES=${libName}"
        "-DIS_NEED_SUCCESS:BOOL=OFF"
    )
endblock()
