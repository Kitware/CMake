static cmVS7FlagTable cmVS10CudaFlagTable[] = {
  // Collect options meant for the host compiler.
  { "AdditionalCompilerOptions", "Xcompiler=", "Host compiler options", "",
    cmVS7FlagTable::UserValue | cmVS7FlagTable::SpaceAppendable },
  { "AdditionalCompilerOptions", "Xcompiler", "Host compiler options", "",
    cmVS7FlagTable::UserFollowing | cmVS7FlagTable::SpaceAppendable },

  { 0, 0, 0, 0, 0 }
};
