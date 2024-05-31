enable_language(C)

# Normally CMAKE_PROJECT_TOP_LEVEL_INCLUDES must be set before the first
# project() call. We don't care about the variable's usual effects here, we
# only care whether the variable is propagated to try_compile() project calls.
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
  ${CMAKE_CURRENT_LIST_DIR}/include_error.cmake
)

try_compile(result
  PROJECT TestProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/proj
  LOG_DESCRIPTION "Project without property set."
)

set_property(GLOBAL PROPERTY PROPAGATE_TOP_LEVEL_INCLUDES_TO_TRY_COMPILE YES)
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
  ${CMAKE_CURRENT_LIST_DIR}/include_pass1.cmake
  ${CMAKE_CURRENT_LIST_DIR}/include_pass2.cmake
)
try_compile(result
  PROJECT TestProject
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/proj
  LOG_DESCRIPTION "Project with property set."
)

# Confirm the property only affects whole project signature
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
  ${CMAKE_CURRENT_LIST_DIR}/include_error.cmake
)
try_compile(result
  SOURCES ${CMAKE_CURRENT_LIST_DIR}/src.c
  LOG_DESCRIPTION "Source file with property set."
)
