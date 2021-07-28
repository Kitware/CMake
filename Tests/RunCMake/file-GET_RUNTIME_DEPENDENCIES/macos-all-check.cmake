function(set_with_libsystem var)
  set(_tmp "${ARGN}")
  if(EXISTS "/usr/lib/libSystem.B.dylib")
    list(APPEND _tmp [[/usr/lib/libSystem\.B\.dylib]])
  endif()
  set("${var}" "${_tmp}" PARENT_SCOPE)
endfunction()

set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/bin/../lib/executable_path/libexecutable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/bin/../lib/rpath_executable_path/librpath_executable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
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

set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
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

set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
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

set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/bin/../lib/executable_path/libexecutable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/bin/../lib/rpath_executable_path/librpath_executable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
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

set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/bundle_executable/bin/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
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

set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/bundle_executable/bin/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
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

# Weak library reference should have exactly the same dependencies as a regular library reference (test 1)
set_with_libsystem(_check
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/bin/../lib/executable_path/libexecutable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/bin/../lib/rpath_executable_path/librpath_executable_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/libtestlib\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/loader_path/libloader_path\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/../rpath/librpath\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/normal/libnormal\.dylib]]
  [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/rpath_loader_path/librpath_loader_path\.dylib]]
  )
check_contents(deps/deps7.txt "^${_check}$")

set(_check
  [[@executable_path/../lib/executable_path_bundle/libexecutable_path_bundle\.dylib]]
  [[@loader_path/loader_path_unresolved/libloader_path_unresolved\.dylib]]
  [[@rpath/librpath_executable_path_bundle\.dylib]]
  [[@rpath/librpath_loader_path_unresolved\.dylib]]
  [[@rpath/librpath_unresolved\.dylib]]
  )
check_contents(deps/udeps7.txt "^${_check}$")

set(_check
  "^libconflict\\.dylib:[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/conflict/libconflict\\.dylib;[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/macos-build/root-all/executable/lib/conflict2/libconflict\\.dylib\n$"
  )
check_contents(deps/cdeps1.txt "${_check}")
check_contents(deps/cdeps2.txt "${_check}")
check_contents(deps/cdeps3.txt "${_check}")
check_contents(deps/cdeps4.txt "${_check}")
check_contents(deps/cdeps5.txt "${_check}")
check_contents(deps/cdeps6.txt "${_check}")
check_contents(deps/cdeps7.txt "${_check}")
