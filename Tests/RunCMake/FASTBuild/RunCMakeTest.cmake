include(RunCMake)

# Unity of size 1 doesn't make sense and shouldn't be created.
run_cmake(Unity1)
run_cmake(Unity2)
run_cmake(UnityBatchSize)
run_cmake(UnityGroup)
run_cmake(UnityIsolate)
run_cmake(UnityIsolateFileSet)
run_cmake(UnitySourceProperties)
run_cmake(UnityFileSetProperties)
run_cmake(UnitySubdirs)
run_cmake(UnitySubdirsIsolate)
run_cmake(DisableCaching)
run_cmake(DisableDistribution)
run_cmake(CachePlugin)
run_cmake(CacheModeInvalid)
block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CacheMode-build)
  # FASTBuild does not cache objects compiled by MSVC with /Zi,
  # which the default Debug configuration uses.
  set(RunCMake_TEST_OPTIONS
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_FASTBUILD_CACHE_MODE=rw
    "-DCMAKE_FASTBUILD_CACHE_PATH=${RunCMake_TEST_BINARY_DIR}/cache")
  run_cmake(CacheMode)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CacheMode-build ${CMAKE_COMMAND} --build .)
endblock()
run_cmake(SetCompilerProps)
run_cmake(ImportedObjectLib)
run_cmake(InterfaceWithPrivateSource)
run_cmake(ObjectAndSharedLibs)
set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/ObjectAndSharedLibs-build)
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(ObjectAndSharedLibs-build ${CMAKE_COMMAND} --build .)

block()
  set(RunCMake_TEST_BINARY_DIR
    ${RunCMake_BINARY_DIR}/CustomCommandDependsArtifact-build)
  unset(RunCMake_TEST_NO_CLEAN)
  run_cmake(CustomCommandDependsArtifact)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(CustomCommandDependsArtifact-build
    ${CMAKE_COMMAND} --build . --target consumer)
endblock()
