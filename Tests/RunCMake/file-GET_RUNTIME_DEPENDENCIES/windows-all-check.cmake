if(CMAKE_C_COMPILER_ID MATCHES "^(Borland|OrangeC)$")
  # Borland upper-cases dll names referenced in import libraries.
  set(conflict_dll [[CONFLICT\.DLL]])
  set(unresolved_dll [[UNRESOLVED\.DLL]])
else()
  set(conflict_dll [[conflict\.dll]])
  set(unresolved_dll [[unresolved\.dll]])
endif()

set(_check
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/\.conflict/\.\./(lib)?libdir\.dll]=]
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/\.search/(lib)?search\.dll]=]
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/(lib)?MixedCase\.dll]=]
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/(lib)?testlib\.dll]=]
  )
check_contents(deps/deps1.txt "^${_check}$")
check_contents(deps/deps2.txt "^${_check}$")
check_contents(deps/deps3.txt "^${_check}$")
set(_check
  "(lib)?${unresolved_dll}"
  )
check_contents(deps/udeps1.txt "^${_check}$")
check_contents(deps/udeps2.txt "^${_check}$")
check_contents(deps/udeps3.txt "^${_check}$")
set(_check
  "^(lib)?${conflict_dll}:[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/\\.conflict/(lib)?conflict\\.dll;[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/(lib)?conflict\\.dll\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
