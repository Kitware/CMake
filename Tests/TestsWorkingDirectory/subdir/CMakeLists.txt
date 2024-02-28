file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}" _real_current_binary_dir)

add_test(NAME WorkingDirectory-Subdir0 COMMAND WorkingDirectory "${_real_current_binary_dir}")

add_test(NAME WorkingDirectory-Subdir1 COMMAND WorkingDirectory "${_real_current_binary_dir}")
set_tests_properties(WorkingDirectory-Subdir1 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${_real_current_binary_dir}")

add_test(NAME WorkingDirectory-Subdir2 COMMAND WorkingDirectory "${_parent_dir}")
set_tests_properties(WorkingDirectory-Subdir2 PROPERTIES
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
)

set(_default_cwd "${_real_current_binary_dir}")

# FIXME: How to deal with /debug, /release, etc. with VS or Xcode?
if(${CMAKE_GENERATOR} MATCHES "Makefiles")
add_test(WorkingDirectory-Subdir3 ${EXECUTABLE_OUTPUT_PATH}/WorkingDirectory ${_default_cwd})
endif()

add_test(NAME WorkingDirectory-Subdir4 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMAND WorkingDirectory ${_real_current_binary_dir})

string(REGEX REPLACE "/[^/]*$" "" _parent_dir "${_real_current_binary_dir}")

add_test(NAME WorkingDirectory-Subdir5 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/.. COMMAND WorkingDirectory ${_parent_dir})

# FIXME: How to deal with /debug, /release, etc. with VS or Xcode?
if(${CMAKE_GENERATOR} MATCHES "Makefiles")
add_test(WorkingDirectory-Subdir6 ${EXECUTABLE_OUTPUT_PATH}/WorkingDirectory ${_default_cwd} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..)
endif()
