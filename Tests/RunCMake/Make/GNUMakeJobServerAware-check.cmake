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

check("CMakeFiles/dummy.dir/build.make" [[\+\$\(CMAKE_COMMAND\) -E true]])
check("CMakeFiles/dummy2.dir/build.make" [[\+\$\(CMAKE_COMMAND\) -E true]])

check("CMakeFiles/dummy3.dir/build.make" [[\+cd (/d )?"?.*"? && \$\(CMAKE_COMMAND\) -E true]])
check("CMakeFiles/dummy4.dir/build.make" [[\+cd (/d )?"?.*"? && \$\(CMAKE_COMMAND\) -E true]])
