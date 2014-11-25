include(RunCMake)

if (RunCMake_GENERATOR MATCHES "Visual Studio")
    set(RunCMake-stderr-file CompileOptions-stderr-VS.txt)
    run_cmake(CompileOptions)
endif()
