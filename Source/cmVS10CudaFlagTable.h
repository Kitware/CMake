static cmVS7FlagTable cmVS10CudaFlagTable[] = {
  // Collect options meant for the host compiler.
  { "AdditionalCompilerOptions", "Xcompiler=", "Host compiler options", "",
    cmVS7FlagTable::UserValue | cmVS7FlagTable::SpaceAppendable },
  { "AdditionalCompilerOptions", "Xcompiler", "Host compiler options", "",
    cmVS7FlagTable::UserFollowing | cmVS7FlagTable::SpaceAppendable },

  // Select the CUDA runtime library.
  { "CudaRuntime", "cudart=none", "No CUDA runtime library", "None", 0 },
  { "CudaRuntime", "cudart=shared", "Shared/dynamic CUDA runtime library",
    "Shared", 0 },
  { "CudaRuntime", "cudart=static", "Static CUDA runtime library", "Static",
    0 },
  { "CudaRuntime", "cudart", "CUDA runtime library", "",
    cmVS7FlagTable::UserFollowing },

  { 0, 0, 0, 0, 0 }
};
