include(RunCMake)

# Unity of size 1 doesn't make sense and shouldn't be created.
run_cmake(Unity1)
run_cmake(Unity2)
run_cmake(UnityBatchSize)
run_cmake(UnityGroup)
run_cmake(UnityIsolate)
run_cmake(DisableCaching)
run_cmake(DisableDistribution)
run_cmake(SetCompilerProps)
run_cmake(ImportedObjectLib)
