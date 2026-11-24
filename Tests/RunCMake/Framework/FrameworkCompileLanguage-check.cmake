file(READ "${RunCMake_TEST_BINARY_DIR}/compile_commands.json" compile_commands)
string(JSON command ERROR_VARIABLE err GET "${compile_commands}" 0 command)
if(err)
  # FASTBuild lists the arguments instead of one command line.
  string(JSON command GET "${compile_commands}" 0 arguments)
endif()

foreach(dir IN ITEMS fw_c fw_cxx)
  # Accept -F"<path>" and "-F<path>", whose path can contain spaces, or -F<path>.
  string(REGEX MATCHALL "(-F\"[^\"]*|\"-F[^\"]*|-F[^ \"]*)/${dir}[\" ]"
    matches "${command} ")
  list(LENGTH matches count)
  if(NOT count EQUAL 1)
    string(APPEND RunCMake_TEST_FAILED
      "Expected exactly one -F flag for ${dir}, found ${count} in:\n  ${command}\n")
  endif()
endforeach()
