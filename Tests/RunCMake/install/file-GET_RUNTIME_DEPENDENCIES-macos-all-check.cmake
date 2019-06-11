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
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/bin/../lib/executable_path/libexecutable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/bin/../lib/rpath_executable_path/librpath_executable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  [[/usr/lib/libSystem\.B\.dylib]]
  )
check_contents(deps/deps1.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps1.txt "^${_check}$")

set(_check
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  [[/usr/lib/libSystem\.B\.dylib]]
  )
check_contents(deps/deps2.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path/libexecutable_path\.dylib]]
  [[@executable_path/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[@executable_path/../lib/executable_path_postexcluded/libexecutable_path_postexcluded\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_executable_path_postexcluded\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps2.txt "^${_check}$")

set(_check
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  [[/usr/lib/libSystem\.B\.dylib]]
  )
check_contents(deps/deps3.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path/libexecutable_path\.dylib]]
  [[@executable_path/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[@executable_path/../lib/executable_path_postexcluded/libexecutable_path_postexcluded\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_executable_path_postexcluded\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps3.txt "^${_check}$")

set(_check
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/bin/../lib/executable_path/libexecutable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/bin/../lib/rpath_executable_path/librpath_executable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  [[/usr/lib/libSystem\.B\.dylib]]
  )
check_contents(deps/deps4.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps4.txt "^${_check}$")

set(_check
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/bundle_executable/bin/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  [[/usr/lib/libSystem\.B\.dylib]]
  )
check_contents(deps/deps5.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path/libexecutable_path\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps5.txt "^${_check}$")

set(_check
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/bundle_executable/bin/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  [[/usr/lib/libSystem\.B\.dylib]]
  )
check_contents(deps/deps6.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path/libexecutable_path\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps6.txt "^${_check}$")

set(_check
  "^libconflict\\.dylib:[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/conflict/libconflict\\.dylib;[^;]*/Tests/RunCMake/install/file-GET_RUNTIME_DEPENDENCIES-macos-build/root-all/executable/lib/conflict2/libconflict\\.dylib\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
check_contents(deps/cdeps4.txt "${_check}")
check_contents(deps/cdeps5.txt "${_check}")
check_contents(deps/cdeps6.txt "${_check}")
