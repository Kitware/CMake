include(RunCMake)

if (RunCMake_GENERATOR STREQUAL "Xcode")
    set(RunCMake-stderr-file IncludeDirectories-stderr-Xcode.txt)
    run_cmake(IncludeDirectories)
elseif (RunCMake_GENERATOR MATCHES "Visual Studio")
    set(RunCMake-stderr-file IncludeDirectories-stderr-VS.txt)
    run_cmake(IncludeDirectories)
endif()
