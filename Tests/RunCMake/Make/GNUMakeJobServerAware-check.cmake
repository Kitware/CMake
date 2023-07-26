set(BUILD_DIR "${RunCMake_BINARY_DIR}/GNUMakeJobServerAware-build")

function(check target line)
  # Read the file and split it into a list of lines
  file(READ ${BUILD_DIR}/${target} contents)
  STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
  STRING(REGEX REPLACE "\n" ";" contents "${contents}")

  set(found FALSE)
  foreach(entry ${contents})
    if("${entry}" MATCHES "${line}")
      set(found TRUE)
      break()
    endif()
  endforeach()

  if(NOT found)
    message(FATAL_ERROR "Could not find '${line}' in ${BUILD_DIR}/${target}\n${contents}")
  endif()
endfunction()

check("CMakeFiles/dummy.dir/build.make" [[\+\$\(CMAKE_COMMAND\) -E true]])
check("CMakeFiles/dummy2.dir/build.make" [[\+\$\(CMAKE_COMMAND\) -E true]])

check("CMakeFiles/dummy3.dir/build.make" [[\+cd (/d )?"?.*"? && \$\(CMAKE_COMMAND\) -E true]])
check("CMakeFiles/dummy4.dir/build.make" [[\+cd (/d )?"?.*"? && \$\(CMAKE_COMMAND\) -E true]])
