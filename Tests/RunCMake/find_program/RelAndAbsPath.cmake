# testNoSuchFile should only be found if the file absolute path is
# incorrectly prepended with the search path.

function(strip_windows_path_prefix p outvar)
    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        string(REGEX REPLACE "^.:" "" p "${p}")
    endif()
    set(${outvar} "${p}" PARENT_SCOPE)
endfunction()

strip_windows_path_prefix("${CMAKE_CURRENT_SOURCE_DIR}" srcdir)

configure_file(testCWD "tmp${srcdir}/testNoSuchFile" COPYONLY)

find_program(PROG_ABS
  NAMES "${srcdir}/testNoSuchFile"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/tmp"
  NO_DEFAULT_PATH
  )
message(STATUS "PROG_ABS='${PROG_ABS}'")

find_program(PROG_ABS_NPD
  NAMES "${srcdir}/testNoSuchFile"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/tmp"
  NAMES_PER_DIR
  NO_DEFAULT_PATH
  )
message(STATUS "PROG_ABS_NPD='${PROG_ABS_NPD}'")

# ./testCWD should not be found without '.' being in the path list.

configure_file(testCWD testCWD COPYONLY)

find_program(PROG_CWD
  NAMES testCWD
  NO_DEFAULT_PATH
  )
message(STATUS "PROG_CWD='${PROG_CWD}'")


set(CMAKE_PREFIX_PATH ".")
# On some platforms / dashboards the current working
# directory can be in PATH or other search locations
# so disable all searching to make sure this fails
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_USE_PACKAGE_ROOT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
find_program(PROG_CWD
  NAMES testCWD
  )
message(STATUS "PROG_CWD='${PROG_CWD}'")

set(CMAKE_PREFIX_PATH ".")
set(CMAKE_FIND_USE_CMAKE_PATH ON)
find_program(PROG_CWD
  NAMES testCWD
  )
message(STATUS "PROG_CWD='${PROG_CWD}'")

find_program(PROG_CWD_NPD
  NAMES testCWD
  NAMES_PER_DIR
  NO_DEFAULT_PATH
  )
message(STATUS "PROG_CWD_NPD='${PROG_CWD_NPD}'")

# Confirm that adding '.' to path does locate ./testCWD.

find_program(PROG_CWD_DOT
  NAMES testCWD
  PATHS .
  NO_DEFAULT_PATH
  )
message(STATUS "PROG_CWD_DOT='${PROG_CWD_DOT}'")

find_program(PROG_CWD_DOT_NPD
  NAMES testCWD
  PATHS .
  NAMES_PER_DIR
  NO_DEFAULT_PATH
  )
message(STATUS "PROG_CWD_DOT_NPD='${PROG_CWD_DOT_NPD}'")
