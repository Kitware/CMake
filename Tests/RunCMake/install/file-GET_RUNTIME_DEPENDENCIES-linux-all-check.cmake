function(check_contents filename contents_regex)
  if(EXISTS "${CMAKE_INSTALL_PREFIX}/${filename}")
    file(READ "${CMAKE_INSTALL_PREFIX}/${filename}" contents)
    if(NOT contents MATCHES "${contents_regex}")
      string(APPEND RunCMake_TEST_FAILED "File contents:
  ${contents}
do not match what we expected:
  ${contents_regex}
in file:
  ${CMAKE_INSTALL_PREFIX}/${filename}\n")
      set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    endif()
  else()
    string(APPEND RunCMake_TEST_FAILED "File ${CMAKE_INSTALL_PREFIX}/${filename} does not exist")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

set(_check
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/libtest_rpath\.so]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/libtest_runpath\.so]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/rpath/librpath\.so]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/rpath_parent/librpath_parent\.so]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/rpath_search/librpath_search\.so]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/runpath/librunpath\.so]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/runpath_search/librunpath_search\.so]]
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
  "^libconflict\\.so:[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/conflict/libconflict\\.so;[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-linux-build/root-all/lib/conflict2/libconflict\\.so\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
