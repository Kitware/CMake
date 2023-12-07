set(BUILD_DIR "${RunCMake_BINARY_DIR}/GNUMakeJobServerAware-build")

function(check target regex)
  file(STRINGS ${BUILD_DIR}/${target} lines
    REGEX ${regex}
  )

  list(LENGTH lines len)
  if(len EQUAL 0)
    message(FATAL_ERROR "Could not find matching lines '${regex}' in ${BUILD_DIR}/${target}")
  endif()
endfunction()

check("/CMakeFiles/Foo.dir/build.make" [[\+cd (/d )?"?.*"? && "?.*"? --build "?.*"?]])
check("/CMakeFiles/Foo.dir/build.make" [[\+cd (/d )?"?.*"? && "?.*"? -E touch "?.*"?]])
check("/CMakeFiles/Foo.dir/build.make" [[\+"?.*"? -E true]])
