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
  [=[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-windows-build/root-all/bin/\.conflict/\.\./(lib)?libdir\.dll]=]
  [=[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-windows-build/root-all/bin/\.search/(lib)?search\.dll]=]
  [=[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-windows-build/root-all/bin/(lib)?testlib\.dll]=]
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
  "^(lib)?conflict\\.dll:[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-windows-build/root-all/bin/\\.conflict/(lib)?conflict\\.dll;[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-windows-build/root-all/bin/(lib)?conflict\\.dll\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
