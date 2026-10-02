enable_language(C)

# Generators that can depend on individual files prepare a generated file by
# running only the custom command that produces it.
#
# The Makefile generators build the whole target instead, so its sources must
# compile there.
if(CMAKE_GENERATOR MATCHES "Make")
  set(generated_target_source "${CMAKE_CURRENT_LIST_DIR}/lib.c")
else()
  set(generated_target_source "${CMAKE_CURRENT_LIST_DIR}/neverbuild.c")
endif()

# Add a custom command that generates the named file in the current binary
# directory and store its full path in the given variable.
function(add_generated_file var name)
  set(file "${CMAKE_CURRENT_BINARY_DIR}/${name}")
  add_custom_command(OUTPUT "${file}"
    COMMAND ${CMAKE_COMMAND} -E touch "${file}"
  )
  set(${var} "${file}" PARENT_SCOPE)
endfunction()
