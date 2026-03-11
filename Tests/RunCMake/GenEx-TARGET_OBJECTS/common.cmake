function(check_target_objects name genex)
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}_vars_$<CONFIG>.cmake" CONTENT "
set(EXPECTED [==[${ARGN}]==])
set(ACTUAL [==[${genex}]==])
")
  add_custom_target(${name}_check ALL
    COMMAND ${CMAKE_COMMAND}
      "-DVARS_FILE=${CMAKE_CURRENT_BINARY_DIR}/${name}_vars_$<CONFIG>.cmake"
      -P "${CMAKE_SOURCE_DIR}/check_target_objects.cmake"
    VERBATIM
    )
endfunction()
