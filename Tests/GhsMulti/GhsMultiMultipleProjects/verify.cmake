# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

function(verify_project_top name)
  unset(fileName CACHE)
  find_file (fileName ${name}.top.gpj
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/sub
    ${CMAKE_CURRENT_BINARY_DIR}/examples
    )

  if (fileName)
    message("Found target ${name}: ${fileName}")
  else()
    message(SEND_ERROR "Could not find project ${name}: ${fileName}")
  endif()
endfunction()

function(verify_exe_built name)
  unset(fileName CACHE)
  find_file (fileName ${name}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/sub
    )

  if (fileName)
    message("Found target ${name}: ${fileName}")
  else()
    message(SEND_ERROR "Could not find project ${name}: ${fileName}")
  endif()
endfunction()

#test project top files were generated
verify_project_top(test)
verify_project_top(test2)
verify_project_top(test3)
verify_exe_built(exe1)
verify_exe_built(exe2)
