include(RunCMake)

run_cmake_script(Basic-NEW)
run_cmake_script(Basic-OLD)
run_cmake_script(Escape2-NEW)
run_cmake_script(AllNEW-Forward)
run_cmake_script(NEW-calls-OLD)
run_cmake_script(OLD-calls-NEW)
run_cmake_script(MixedChain-OLD-NEWMiddle)
run_cmake_script(DynamicDispatch-NEW)
run_cmake_script(ParseArguments-Semicolon)
run_cmake_script(SemicolonEscape-QuotedUnquoted)
run_cmake_script(ReturnAndPolicyPropagation)
run_cmake_script(Warn-Unset-Macro)
run_cmake_script(Warn-Unset-Macro-Multi)
run_cmake_script(Warn-Unset-VariableWatch)
run_cmake_script(VariableWatch-OLD)
run_cmake_script(VariableWatch-NEW)
run_cmake(CheckCSourceCompiles-OLD)
run_cmake(CheckCSourceCompiles-NEW)
run_cmake_with_options(DependencyProviderMacro-OLD
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=${RunCMake_SOURCE_DIR}/DependencyProviderMacro-TopInclude.cmake"
  -D "CMAKE_POLICY_DEFAULT_CMP0219=OLD"
)
run_cmake_with_options(DependencyProviderMacro-NEW
  -D "CMAKE_PROJECT_TOP_LEVEL_INCLUDES=${RunCMake_SOURCE_DIR}/DependencyProviderMacro-TopInclude.cmake"
  -D "CMAKE_POLICY_DEFAULT_CMP0219=NEW"
)
run_cmake(Defer-EndPolicy-NEW)
run_cmake(Defer-EndPolicy-OLD)
