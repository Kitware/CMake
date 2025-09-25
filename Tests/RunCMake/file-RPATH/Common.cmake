if(CMAKE_HOST_UNIX)
  function(permissions_set f)
    file(CHMOD ${f} FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE SETUID)
  endfunction()
  function(permissions_check f)
    execute_process(COMMAND sh -c "stat -c %a \"${f}\""
      OUTPUT_VARIABLE stat_out OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_VARIABLE stat_err
      RESULT_VARIABLE stat_res
      )
    if(stat_res EQUAL 0)
      if(NOT stat_out STREQUAL "4700")
        message(FATAL_ERROR "Expected permissions 4700 but got ${stat_out}:\n ${f}")
      endif()
    endif()
  endfunction()
else()
  function(permissions_set)
  endfunction()
  function(permissions_check)
  endfunction()
endif()

# Prepare binaries on which to operate.
set(in "${CMAKE_CURRENT_LIST_DIR}/${format}")
set(out "${CMAKE_CURRENT_BINARY_DIR}")
foreach(f ${dynamic})
  file(COPY ${in}/${f} DESTINATION ${out} NO_SOURCE_PERMISSIONS)
  list(APPEND dynamic_files "${out}/${f}")
endforeach()
foreach(f ${static})
  file(COPY ${in}/${f} DESTINATION ${out} NO_SOURCE_PERMISSIONS)
  list(APPEND static_files "${out}/${f}")
endforeach()

foreach(f ${dynamic_files})
  permissions_set(${f})

  # Check for the initial RPATH.
  file(RPATH_CHECK FILE "${f}" RPATH "/sample/rpath")
  if(NOT EXISTS "${f}")
    message(FATAL_ERROR "RPATH_CHECK removed ${f}")
  endif()
  permissions_check(${f})

  # Change the RPATH.
  file(RPATH_CHANGE FILE "${f}"
    OLD_RPATH "/sample/rpath"
    NEW_RPATH "/path1:/path2")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/path1:/path2" LIMIT_COUNT 1)
  if(NOT rpath)
    message(FATAL_ERROR "RPATH not changed in ${f}")
  endif()
  permissions_check(${f})

  # Change the RPATH without compiler defined rpath removed
  file(RPATH_CHANGE FILE "${f}"
    OLD_RPATH "/path2"
    NEW_RPATH "/path3")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/path1:/path3" LIMIT_COUNT 1)
  if(NOT rpath)
    message(FATAL_ERROR "RPATH not updated in ${f}")
  endif()
  permissions_check(${f})

  # Change the RPATH with compiler defined rpath removed
  file(RPATH_CHANGE FILE "${f}"
    OLD_RPATH "/path3"
    NEW_RPATH "/rpath/sample"
    INSTALL_REMOVE_ENVIRONMENT_RPATH)
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/rpath/sample" LIMIT_COUNT 1)
  if(NOT rpath)
    message(FATAL_ERROR "RPATH not updated in ${f}")
  endif()
  file(STRINGS "${f}" rpath REGEX "/path1" LIMIT_COUNT 1)
  if(rpath)
    message(FATAL_ERROR "RPATH not removed in ${f}")
  endif()
  permissions_check(${f})

  # Remove the RPATH.
  file(RPATH_REMOVE FILE "${f}")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/rpath/sample" LIMIT_COUNT 1)
  if(rpath)
    message(FATAL_ERROR "RPATH not removed from ${f}")
  endif()
  permissions_check(${f})

  # Check again...this should remove the file.
  file(RPATH_CHECK FILE "${f}" RPATH "/sample/rpath")
  if(EXISTS "${f}")
    message(FATAL_ERROR "RPATH_CHECK did not remove ${f}")
  endif()
endforeach()

# TODO Implement RPATH_SET in XCOFF.
if(format STREQUAL "ELF")
  foreach(f ${dynamic})
    file(COPY ${in}/${f} DESTINATION ${out} NO_SOURCE_PERMISSIONS)
  endforeach()

  foreach(f ${dynamic_files})
    # Set the RPATH.
    file(RPATH_SET FILE "${f}"
      NEW_RPATH "/new/rpath")
    set(rpath)
    file(STRINGS "${f}" rpath REGEX "/new/rpath" LIMIT_COUNT 1)
    if(NOT rpath)
      message(FATAL_ERROR "RPATH not set in ${f}")
    endif()
    file(STRINGS "${f}" rpath REGEX "/rpath/sample" LIMIT_COUNT 1)
    if(rpath)
      message(FATAL_EROR "RPATH not removed in ${f}")
    endif()

    # Remove the RPATH.
    file(RPATH_SET FILE "${f}"
      NEW_RPATH "")
    set(rpath)
    file(STRINGS "${f}" rpath REGEX "/new/rpath" LIMIT_COUNT 1)
    if(rpath)
      message(FATAL_ERROR "RPATH not removed from ${f}")
    endif()

    # Check again...this should remove the file.
    file(RPATH_CHECK FILE "${f}" RPATH "/new/rpath")
    if(EXISTS "${f}")
      message(FATAL_ERROR "RPATH_CHECK did not remove ${f}")
    endif()
  endforeach()
endif()

# Verify that modifying rpaths on a static library is a no-op
foreach(f ${static_files})
  file(RPATH_CHANGE FILE "${f}" OLD_RPATH "/rpath/foo" NEW_RPATH "/rpath/bar")
endforeach()
