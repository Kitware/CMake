enable_language(C)
if(FAIL)
  message(FATAL_ERROR "Intentionally fail to configure")
endif()

find_file(find_file_var # first search
  NAMES "configureLog-v1.cmake"
  PATHS "${CMAKE_CURRENT_LIST_DIR}"
  DOC   "find_file search")
find_file(find_file_var # re-find (no log)
  NAMES "configureLog-v1.cmake"
  PATHS "${CMAKE_CURRENT_LIST_DIR}"
  DOC   "find_file search")
unset(find_file_var)
set(find_file_var "find_file_var-NOTFOUND" CACHE PATH "" FORCE)
find_file(find_file_var # not-found to found
  NAMES "configureLog-v1.cmake"
  PATHS "${CMAKE_CURRENT_LIST_DIR}"
  DOC   "find_file search")

find_path(find_path_var # first search
  NAMES "configureLog-v1.cmake"
  PATHS "${CMAKE_CURRENT_LIST_DIR}"
  DOC   "find_path search")
find_path(find_path_var # re-find (no log)
  NAMES "configureLog-v1.cmake"
  PATHS "${CMAKE_CURRENT_LIST_DIR}"
  DOC   "find_path search")
unset(find_path_var)
set(find_path_var "find_path_var-NOTFOUND" CACHE PATH "" FORCE)
find_path(find_path_var # not-found to found
  NAMES "configureLog-v1.cmake"
  PATHS "${CMAKE_CURRENT_LIST_DIR}"
  DOC   "find_path search")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lib${CMAKE_SHARED_LIBRARY_SUFFIX}")
find_library(find_library_var # first search
  NAMES "lib"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
  DOC   "find_library search")
find_library(find_library_var # re-find (no log)
  NAMES "lib"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
  DOC   "find_library search")
unset(find_library_var)
set(find_library_var "find_library_var-NOTFOUND" CACHE PATH "" FORCE)
find_library(find_library_var # not-found to found
  NAMES "lib"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
  DOC   "find_library search")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/exe${CMAKE_EXECUTABLE_SUFFIX}" "")
file(CHMOD  "${CMAKE_CURRENT_BINARY_DIR}/exe${CMAKE_EXECUTABLE_SUFFIX}"
  FILE_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ             GROUP_EXECUTE
    WORLD_READ             WORLD_EXECUTE)
find_program(find_program_var # first search
  NAMES "exe"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
  DOC   "find_program search")
find_program(find_program_var # re-find (no log)
  NAMES "exe"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
  DOC   "find_program search")
unset(find_program_var)
set(find_program_var "find_program_var-NOTFOUND" CACHE PATH "" FORCE)
find_program(find_program_var # not-found to found
  NAMES "exe"
  PATHS "${CMAKE_CURRENT_BINARY_DIR}"
  DOC   "find_program search")
