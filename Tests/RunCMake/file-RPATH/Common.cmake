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
  # Check for the initial RPATH.
  file(RPATH_CHECK FILE "${f}" RPATH "/sample/rpath")
  if(NOT EXISTS "${f}")
    message(FATAL_ERROR "RPATH_CHECK removed ${f}")
  endif()

  # Change the RPATH.
  file(RPATH_CHANGE FILE "${f}"
    OLD_RPATH "/sample/rpath"
    NEW_RPATH "/path1:/path2")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/path1:/path2" LIMIT_COUNT 1)
  if(NOT rpath)
    message(FATAL_ERROR "RPATH not changed in ${f}")
  endif()

  # Change the RPATH without compiler defined rpath removed
  file(RPATH_CHANGE FILE "${f}"
    OLD_RPATH "/path2"
    NEW_RPATH "/path3")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/path1:/path3" LIMIT_COUNT 1)
  if(NOT rpath)
    message(FATAL_ERROR "RPATH not updated in ${f}")
  endif()

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

  # Remove the RPATH.
  file(RPATH_REMOVE FILE "${f}")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/rpath/sample" LIMIT_COUNT 1)
  if(rpath)
    message(FATAL_ERROR "RPATH not removed from ${f}")
  endif()

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
