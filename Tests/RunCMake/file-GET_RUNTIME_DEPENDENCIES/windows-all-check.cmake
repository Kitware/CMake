set(_check
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/\.conflict/\.\./(lib)?libdir\.dll]=]
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/\.search/(lib)?search\.dll]=]
  [=[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/(lib)?testlib\.dll]=]
  )
check_contents(deps/deps1.txt "^${_check}$")
check_contents(deps/deps2.txt "^${_check}$")
check_contents(deps/deps3.txt "^${_check}$")
set(_check
  [=[(lib)?unresolved\.dll]=]
  )
check_contents(deps/udeps1.txt "^${_check}$")
check_contents(deps/udeps2.txt "^${_check}$")
check_contents(deps/udeps3.txt "^${_check}$")
set(_check
  "^(lib)?conflict\\.dll:[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/\\.conflict/(lib)?conflict\\.dll;[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/windows-build/root-all/bin/(lib)?conflict\\.dll\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
