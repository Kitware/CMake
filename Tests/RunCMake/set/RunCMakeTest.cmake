include(RunCMake)

run_cmake(CacheErrors)
run_cmake(ParentScope)
run_cmake(ParentPulling)
run_cmake(ParentPullingRecursive)
run_cmake(UnknownCacheType)
run_cmake(ExtraEnvValue)

# set(CACHE{}) syntax
run_cmake(CacheUnknownArguments)
run_cmake(CacheMissingArguments)
run_cmake(CacheWrongTYPE)
run_cmake(CacheSetUnset)

# set(ENV{}) syntax
run_cmake_script(Env)
