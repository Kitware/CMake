include(RunCMake)
unset(ENV{Boost_ROOT})

run_cmake(CMakePackage)
run_cmake(NoCXX)

run_cmake(LegacyVars-TargetsDefined) # "Good" BoostConfig
run_cmake(LegacyVars-LowercaseTargetPrefix)
set(RunCMake-stdout-file LegacyVars-TargetsDefined-stdout.txt)
run_cmake(LegacyVars-NoHeaderTarget)

unset(RunCMake-stdout-file)
run_cmake(MissingTarget)

set(RunCMake-stdout-file CommonResults-stdout.txt)
run_cmake(ConfigMode)
run_cmake(ModuleMode)
unset(RunCMake-stdout-file)
set(RunCMake-stdout-file CommonNotFound-stdout.txt)
set(RunCMake-stderr-file CommonNotFound-stderr.txt)
run_cmake(ConfigModeNotFound)
run_cmake(ModuleModeNotFound)
unset(RunCMake-stdout-file)
unset(RunCMake-stderr-file)

run_cmake(CMP0093-NEW)
run_cmake(CMP0093-OLD)
run_cmake(CMP0093-UNSET)
