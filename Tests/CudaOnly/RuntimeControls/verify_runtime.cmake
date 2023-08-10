
string(REPLACE "|" ";" dirs "${EXTRA_LIB_DIRS}")
file(GET_RUNTIME_DEPENDENCIES
  RESOLVED_DEPENDENCIES_VAR resolved_libs
  UNRESOLVED_DEPENDENCIES_VAR unresolved_libs
  DIRECTORIES ${dirs}
  EXECUTABLES ${EXEC_PATH}
  )

list(FILTER resolved_libs INCLUDE REGEX ".*[Cc][Uu][Dd][Aa][Rr][Tt].*")
list(LENGTH resolved_libs has_cudart)

if(has_cudart EQUAL 0)
  message(FATAL_ERROR
    "missing cudart shared library from runtime dependency output.")
endif()
