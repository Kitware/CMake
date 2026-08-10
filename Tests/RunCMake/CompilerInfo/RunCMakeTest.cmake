include(RunCMake)

block()
  # The stand-in assembler reports an MSVC-like command-line by default and a
  # GNU-like one when PSEUDO_ASM_GNU is set, which selects the two branches of
  # assembler identification that populate these fields differently.
  set(RunCMake_TEST_OPTIONS "-DCMAKE_ASM_COMPILER=${PSEUDO_ASM}")

  run_cmake(ASM-clang-cl)

  set(ENV{PSEUDO_ASM_GNU} 1)
  run_cmake(ASM-clang-gnu)
  unset(ENV{PSEUDO_ASM_GNU})
endblock()
