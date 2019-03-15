include(RunCMake)

set(RunCMake_GENERATOR "Visual Studio 12 2013")
set(RunCMake_GENERATOR_TOOLSET CE800)
set(RunCMake_GENERATOR_INSTANCE "")
set(RunCMake_TEST_OPTIONS -DCMAKE_SYSTEM_NAME=WindowsCE  )

run_cmake(VsCEDebuggerDeploy)
run_cmake(VSCSharpCFProject)
