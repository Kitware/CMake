
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt"
            "${CMAKE_CURRENT_BINARY_DIR}/writable.txt"
            "${CMAKE_CURRENT_BINARY_DIR}/executable.txt")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt" "foo")
file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/readable.txt" PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/writable.txt" "foo")
file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/writable.txt" PERMISSIONS OWNER_WRITE GROUP_WRITE)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/executable.txt" "foo")
file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/executable.txt" PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)

if(NOT WIN32)
  file(REMOVE_RECURSE
    "${CMAKE_CURRENT_BINARY_DIR}/readable-dir"
    "${CMAKE_CURRENT_BINARY_DIR}/writable-dir"
    "${CMAKE_CURRENT_BINARY_DIR}/executable-dir"
    )

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/readable-dir")
  file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/readable-dir" PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/writable-dir")
  file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/writable-dir" PERMISSIONS OWNER_WRITE GROUP_WRITE)

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/executable-dir")
  file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/executable-dir" PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)
endif()

function(cleanup)
  if(NOT WIN32)
    # CMake versions prior to 3.29 did not know how to remove non-readable directories.
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/writable-dir" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_WRITE)
    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/executable-dir" PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)
  endif()
endfunction()

if(WIN32)
  # files are always readable and executable
  # directories are always, readable, writable and executable
  if(NOT IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt"
      OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/readable.txt\" failed")
  endif()

  if(NOT IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/executable.txt"
      OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/executable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/executable.txt\" failed")
  endif()
else()
  if(NOT IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt"
      OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt"
      OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/readable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/readable.txt\" failed")
  endif()

  if(NOT IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/writable.txt"
      OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/writable.txt"
      OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/writable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/writable.txt\" failed")
  endif()

  if(NOT IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/executable.txt"
      OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/executable.txt"
      OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/executable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/executable.txt\" failed")
  endif()


  if(NOT IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/readable-dir"
      OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/readable-dir"
      OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/readable-dir")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/readable-dir\" failed")
  endif()

  if(NOT IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/writable-dir"
      OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/writable-dir"
      OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/writable-dir")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/writable-dir\" failed")
  endif()

  if(NOT IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/executable-dir"
      OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/executable-dir"
      OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/executable-dir")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/executable.txt\" failed")
  endif()
endif()

if(UNIX)
  #
  # Check that file permissions are on the real file, not the symbolic link
  #
  file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable.txt"
              "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable.txt"
              "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable.txt"
              "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable-dir"
              "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable-dir"
              "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable-dir")

  file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/readable.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable.txt"
    SYMBOLIC)

  file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/writable.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable.txt"
    SYMBOLIC)

  file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/executable.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable.txt"
    SYMBOLIC)


  file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/readable-dir"
    "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable-dir"
    SYMBOLIC)

  file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/writable-dir"
    "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable-dir"
    SYMBOLIC)

  file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/executable-dir"
    "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable-dir"
    SYMBOLIC)

  if(NOT IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable.txt"
     OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable.txt"
     OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/link-to-readable.txt\" failed")
  endif()

  if(NOT IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable.txt"
     OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable.txt"
     OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/link-to-writable.txt\" failed")
  endif()

  if(NOT IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable.txt"
     OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable.txt"
     OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable.txt")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/link-to-executable.txt\" failed")
  endif()


  if(NOT IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable-dir"
     OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable-dir"
     OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-readable-dir")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/link-to-readable-dir\" failed")
  endif()

  if(NOT IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable-dir"
     OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable-dir"
     OR IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-writable-dir")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/link-to-writable-dir\" failed")
  endif()

  if(NOT IS_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable-dir"
     OR IS_READABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable-dir"
     OR IS_WRITABLE "${CMAKE_CURRENT_BINARY_DIR}/link-to-executable-dir")
    cleanup()
    message(FATAL_ERROR "checks on \"${CMAKE_CURRENT_BINARY_DIR}/link-to-executable-dir\" failed")
  endif()
endif()

cleanup()
