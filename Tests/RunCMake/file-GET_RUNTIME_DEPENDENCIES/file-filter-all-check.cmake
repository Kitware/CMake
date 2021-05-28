if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(_check
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep1\.so]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep2\.so]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep3\.so]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep4\.so]]
    )
  check_contents(deps/deps.txt "^${_check}$")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(_check
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep1\.dylib]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep2\.dylib]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep3\.dylib]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/\.\./lib/libdep4\.dylib]]
    )
  check_contents(deps/deps.txt "^${_check}$")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(_check
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/(lib)?dep1\.dll]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/(lib)?dep2\.dll]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/(lib)?dep3\.dll]]
    [[[^;]*/Tests/RunCMake/file-GET_RUNTIME_DEPENDENCIES/file-filter-build/root-all/bin/(lib)?dep4\.dll]]
    )
  check_contents(deps/deps.txt "^${_check}$")
endif()

check_contents(deps/udeps.txt "^$")
check_contents(deps/cdeps.txt "^$")
