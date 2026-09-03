set(vcProjectFile "${RunCMake_TEST_BINARY_DIR}/nasmexe.vcxproj")
if(NOT EXISTS "${vcProjectFile}")
  # No ASM_NASM compiler was available, so there is nothing to verify.
  return()
endif()

file(STRINGS "${vcProjectFile}" linkToolExeLines REGEX "<LinkToolExe>")
if(linkToolExeLines)
  string(REPLACE ";" "\n  " linkToolExeLines "${linkToolExeLines}")
  set(RunCMake_TEST_FAILED
    "nasmexe.vcxproj should not set <LinkToolExe> for an ASM_NASM target; "
    "MSBuild should link via the toolset.  Found:\n  ${linkToolExeLines}")
endif()
