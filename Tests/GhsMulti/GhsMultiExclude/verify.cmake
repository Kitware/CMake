# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

function(verify_skipped_tgt name)
  unset(fileName CACHE)
  find_file (fileName ${name}.tgt.gpj
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  if (fileName)
    message("Found target ${name}: ${fileName}")
  else()
    message(SEND_ERROR "Could not find target ${name}: ${fileName}")
  endif()

  #test project was built
  unset(fileName CACHE)
  find_file (fileName lib${name}.a
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  if (fileName)
    message(SEND_ERROR "Found target ${name}: ${fileName}")
  else()
    message("Could not find target ${name}: ${fileName}")
  endif()
endfunction()

function(locate_tgt name)
  unset(fileName CACHE)
  find_file (fileName ${name}.tgt.gpj
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  if (fileName)
    message("Found target ${name}: ${fileName}")
  else()
    message(SEND_ERROR "Could not find target ${name}: ${fileName}")
  endif()

  #test project was built
  unset(fileName CACHE)
  find_file (fileName lib${name}.a
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  if (fileName)
    message( "Found target ${name}: ${fileName}")
  else()
    message(SEND_ERROR "Could not find target ${name}: ${fileName}")
  endif()
endfunction()

verify_skipped_tgt(lib1)
verify_skipped_tgt(lib2)
locate_tgt(lib3)
