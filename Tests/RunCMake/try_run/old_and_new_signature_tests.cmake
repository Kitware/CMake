# These tests are performed using both the historic and the newer SOURCES
# signatures of try_run. It is critical that they behave the same and produce
# comparable output for both signatures. Tests that cannot do this belong in
# RunCMakeTests.txt, not here.
#
# Tests here MUST include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS}) and
# use the variables defined therein appropriately. Refer to existing tests for
# examples.

run_cmake(BadLinkLibraries)
run_cmake(BinDirEmpty)
run_cmake(BinDirRelative)

run_cmake(CrossCompile)

run_cmake(WorkingDirArg)

run_cmake(NoCompileOutputVariable)
run_cmake(NoRunOutputVariable)
run_cmake(NoRunStdOutVariable)
run_cmake(NoRunStdErrVariable)
run_cmake(NoWorkingDirectory)
