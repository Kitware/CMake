file(GLOB_RECURSE files "${dir}/*")

foreach(file IN LISTS files)
  if(file MATCHES "/(build|install)(no)?ge$")
    file(RPATH_CHANGE FILE "${file}" OLD_RPATH "/opt/foo/lib" NEW_RPATH "/opt/bar/lib")
  endif()
endforeach()
