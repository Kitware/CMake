function(read_json filename outvar)
  file(READ "${filename}" ${outvar})
  return(PROPAGATE ${outvar})
endfunction()
