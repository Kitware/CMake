macro(read_json filename outvar)
  file(READ ${filename} contents)
  # string(JSON *) will fail if JSON file contains any forward-slash paths
  string(REGEX REPLACE "[\\]([a-zA-Z0-9 ])" "/\\1" contents ${contents})
  # string(JSON *) will fail if JSON file contains any escaped quotes \"
  string(REPLACE "\\\"" "'" contents ${contents})
  set(${outvar} ${contents})
endmacro()
