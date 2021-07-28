set(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/libtest_rpath\.so]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/libtest_runpath\.so]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/rpath/librpath\.so]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/rpath_parent/librpath_parent\.so]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/rpath_search/librpath_search\.so]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/runpath/librunpath\.so]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/runpath_search/librunpath_search\.so]]
  )
check_contents(deps/deps1.txt "^${_check}$")
check_contents(deps/deps2.txt "^${_check}$")
check_contents(deps/deps3.txt "^${_check}$")
set(_check
  [[librpath_unresolved\.so]]
  [[librunpath_parent_unresolved\.so]]
  [[librunpath_unresolved\.so]]
  )
check_contents(deps/udeps1.txt "^${_check}$")
check_contents(deps/udeps2.txt "^${_check}$")
check_contents(deps/udeps3.txt "^${_check}$")
set(_check
  "^libconflict\\.so:[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/conflict/libconflict\\.so;[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/linux-build/root-all/lib/conflict2/libconflict\\.so\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
