include(RunCMake)

run_cmake(InterfaceBool-mismatch-depends)
run_cmake(InterfaceBool-mismatch-depend-self)
run_cmake(InterfaceBool-mismatched-use)
run_cmake(InterfaceBool-builtin-prop)
run_cmake(InterfaceString-mismatch-depends)
run_cmake(InterfaceString-mismatch-depend-self)
run_cmake(InterfaceString-mismatched-use)
run_cmake(InterfaceString-builtin-prop)
run_cmake(InterfaceString-Bool-Conflict)
run_cmake(DebugProperties)

set(RunCMake_TEST_OPTIONS -DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE})

run_cmake(AutoUic)
