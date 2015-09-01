function(BuildTargetInSubProject P T E)
  try_compile(OUTVAR
    ${CMAKE_CURRENT_BINARY_DIR}/subproject
    ${CMAKE_CURRENT_SOURCE_DIR}/subproject
    ${P} ${T})
  if(E AND OUTVAR)
    message(STATUS "${P} target ${T} succeeded as expected")
  elseif(E AND NOT OUTVAR)
    message(FATAL_ERROR "${P} target ${T} failed but should have succeeded")
  elseif(NOT E AND NOT OUTVAR)
    message(STATUS "${P} target ${T} failed as expected")
  elseif(NOT E AND OUTVAR)
    message(FATAL_ERROR "${P} target ${T} succeeded but should have failed")
  endif()
endfunction()
